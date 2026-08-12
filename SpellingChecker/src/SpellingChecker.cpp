#include "SpellingChecker.h"

#include <algorithm>
#include <queue>
#include <filesystem>
#include <iostream>

namespace bijoy::spelling {

SpellChecker::SpellChecker() : m_fileSize(0) {}

SpellChecker::~SpellChecker() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

// -------------------------------------------------------------------------
// LoadDictionary
// Only opens the file and records its size. ZERO RAM used for the 7B words.
// file must be alphabetically sorted!
// -------------------------------------------------------------------------
int SpellChecker::LoadDictionary(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_file.is_open()) {
        m_file.close();
    }

    m_file.open(filePath, std::ios::binary | std::ios::in);
    if (!m_file.is_open()) {
        return -1;
    }

    m_file.seekg(0, std::ios::end);
    m_fileSize = m_file.tellg();
    m_file.seekg(0, std::ios::beg);

    return 0; // Success
}

size_t SpellChecker::GetDictionarySize() const {
    return static_cast<size_t>(m_fileSize); // Returning byte size instead of word count
}

// -------------------------------------------------------------------------
// ReadNextWordFromOffset
// Helper to align to the next word boundary and read a full word.
// -------------------------------------------------------------------------
bool SpellChecker::ReadNextWordFromOffset(uintmax_t offset, std::string& outWord, uintmax_t& outStartOffset) {
    if (offset >= m_fileSize) return false;

    m_file.seekg(offset);
    char buf[1];
    
    // If we're not at the beginning, we must skip characters until we see a newline
    if (offset > 0) {
        bool foundNewline = false;
        while (m_file.read(buf, 1)) {
            if (buf[0] == '\n') {
                foundNewline = true;
                break;
            }
        }
        if (!foundNewline) return false; // Reached EOF without finding a word
    }

    outStartOffset = m_file.tellg();
    outWord.clear();

    // Read the word until the next newline or EOF
    while (m_file.read(buf, 1)) {
        if (buf[0] == '\r') continue; // Ignore carriage return
        if (buf[0] == '\n') break;
        outWord.push_back(buf[0]);
    }

    // Skip empty lines or trailing newlines
    if (outWord.empty() && !m_file.eof()) {
        return ReadNextWordFromOffset(outStartOffset + 1, outWord, outStartOffset);
    }
    
    return !outWord.empty();
}

// -------------------------------------------------------------------------
// FindInsertionOffset
// Standard binary search on a disk file. O(log N) Reads.
// Returns the exact start byte of where the word is found, OR where it 
// WOULD be inserted alphabetically.
// -------------------------------------------------------------------------
uintmax_t SpellChecker::FindInsertionOffset(const std::string& word) {
    uintmax_t left = 0;
    uintmax_t right = m_fileSize;
    uintmax_t lastCheckedOffset = 0;

    std::string currentWord;
    uintmax_t currentWordStart = 0;

    while (left < right) {
        uintmax_t mid = left + (right - left) / 2;

        if (!ReadNextWordFromOffset(mid, currentWord, currentWordStart)) {
            // Reached near EOF, clamp right
            right = mid;
            continue;
        }

        lastCheckedOffset = currentWordStart;

        if (currentWord == word) {
            return currentWordStart; // Exact match found
        } 
        
        if (currentWord < word) {
            // word is alphabetically after currentWord
            left = currentWordStart + currentWord.size() + 1;
            if (left >= right) break; // prevent infinite loop
        } else {
            // word is alphabetically before currentWord
            // if we are too close to zero, break early
            if (mid == 0) break;
            right = mid; 
        }
    }
    return lastCheckedOffset;
}

// -------------------------------------------------------------------------
// Check
// -------------------------------------------------------------------------
bool SpellChecker::Check(const std::string& word) {
    if (m_fileSize == 0) return false;
    std::lock_guard<std::mutex> lock(m_mutex);

    uintmax_t offset = FindInsertionOffset(word);
    
    std::string diskWord;
    uintmax_t actualOffset;
    if (ReadNextWordFromOffset(offset, diskWord, actualOffset)) {
        if (diskWord == word) return true;
        
        // Edge case: sometimes binary search lands slightly off. Scan a few words ahead/behind in real scenario, 
        // but binary search usually lands on it perfectly if file is strictly sorted.
        // As a safeguard, read 1-2 words forward to just be completely sure.
        for(int i = 0; i < 3; i++) {
            if (ReadNextWordFromOffset(m_file.tellg(), diskWord, actualOffset)) {
                if (diskWord == word) return true;
                if (diskWord > word) break; // Alphabetically passed it
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------
// Suggest
// Fetches a 100KB buffer around the binary searched target to get locally 
// clustered suggestions with Zero-RAM overhead compared to full scan.
// -------------------------------------------------------------------------
std::vector<std::string> SpellChecker::Suggest(const std::string& word, int maxResults) {
    if (m_fileSize == 0) return {};
    std::lock_guard<std::mutex> lock(m_mutex);

    uintmax_t centerOffset = FindInsertionOffset(word);

    // We want to load a ~100KB "Proximity Chunk" surrounding the word.
    // E.g. 50KB backward, 50KB forward
    const uintmax_t CHUNK_HALF_SIZE = 50 * 1024;
    
    uintmax_t startRead = (centerOffset > CHUNK_HALF_SIZE) ? (centerOffset - CHUNK_HALF_SIZE) : 0;
    uintmax_t readLength = CHUNK_HALF_SIZE * 2;
    if (startRead + readLength > m_fileSize) {
        readLength = m_fileSize - startRead;
    }

    // Read the block into a temporary buffer
    std::string buffer;
    buffer.resize(readLength);
    m_file.seekg(startRead);
    m_file.read(&buffer[0], readLength);
    
    // Parse words from the buffer over memory and run Levenshtein early abort
    auto inputCps = Utf8ToCodepoints(word);
    constexpr int kMaxDist = 3;

    using Candidate = std::pair<int, std::string>;
    auto cmp = [](const Candidate& a, const Candidate& b) {
        return a.first > b.first; 
    };
    std::priority_queue<Candidate, std::vector<Candidate>, decltype(cmp)> heap(cmp);

    size_t pos = 0;
    
    // Skip the first partial word if startRead > 0
    if (startRead > 0) {
        pos = buffer.find('\n');
        if (pos != std::string::npos) pos++;
        else pos = buffer.size();
    }

    while (pos < buffer.size()) {
        size_t nextPos = buffer.find('\n', pos);
        if (nextPos == std::string::npos) nextPos = buffer.size();
        
        size_t endPos = nextPos;
        if (endPos > pos && buffer[endPos - 1] == '\r') endPos--;
        
        std::string entry = buffer.substr(pos, endPos - pos);
        
        if (!entry.empty()) {
            int dist = LevenshteinDistanceEarlyAbort(inputCps, entry, kMaxDist);
            if (dist > 0 && dist <= kMaxDist) {
                heap.push({dist, entry});
            }
        }
        pos = nextPos + 1;
    }

    std::vector<std::string> results;
    results.reserve(static_cast<size_t>(maxResults));

    while (!heap.empty() && static_cast<int>(results.size()) < maxResults) {
        // Prevent duplicates in suggestion
        const auto& candidate = heap.top().second;
        if (std::find(results.begin(), results.end(), candidate) == results.end()) {
            results.push_back(candidate);
        }
        heap.pop();
    }

    return results;
}

// -------------------------------------------------------------------------
// Utf8ToCodepoints (Helper)
// -------------------------------------------------------------------------
std::vector<uint32_t> SpellChecker::Utf8ToCodepoints(const std::string& str) {
    std::vector<uint32_t> codepoints;
    const auto* bytes = reinterpret_cast<const unsigned char*>(str.data());
    size_t len = str.size();
    size_t i = 0;
    codepoints.reserve(len);

    while (i < len) {
        uint32_t cp = 0;
        unsigned char b = bytes[i];

        if (b < 0x80) {
            cp = b;
            i += 1;
        } else if ((b & 0xE0) == 0xC0) {
            cp = b & 0x1F;
            if (i + 1 < len) cp = (cp << 6) | (bytes[i + 1] & 0x3F);
             i += 2;
        } else if ((b & 0xF0) == 0xE0) {
            cp = b & 0x0F;
            if (i + 1 < len) cp = (cp << 6) | (bytes[i + 1] & 0x3F);
            if (i + 2 < len) cp = (cp << 6) | (bytes[i + 2] & 0x3F);
            i += 3;
        } else if ((b & 0xF8) == 0xF0) {
            cp = b & 0x07;
            if (i + 1 < len) cp = (cp << 6) | (bytes[i + 1] & 0x3F);
            if (i + 2 < len) cp = (cp << 6) | (bytes[i + 2] & 0x3F);
            if (i + 3 < len) cp = (cp << 6) | (bytes[i + 3] & 0x3F);
            i += 4;
        } else {
            i += 1;
            continue;
        }
        codepoints.push_back(cp);
    }
    return codepoints;
}

// -------------------------------------------------------------------------
// LevenshteinDistanceEarlyAbort
// -------------------------------------------------------------------------
int SpellChecker::LevenshteinDistanceEarlyAbort(
    const std::vector<uint32_t>& cpA, 
    const std::string& bUtf8,
    int maxDist) 
{
    const size_t lenA = cpA.size();
    int prev[128], curr[128];
    for (size_t j = 0; j <= 127; ++j) prev[j] = static_cast<int>(j);

    const auto* bytes = reinterpret_cast<const unsigned char*>(bUtf8.data());
    size_t lenByteB = bUtf8.size();
    size_t i = 1, byteIdx = 0;
    
    while (byteIdx < lenByteB && i < 127) {
        uint32_t cpB = 0;
        unsigned char b = bytes[byteIdx];
        if (b < 0x80) {
            cpB = b; byteIdx += 1;
        } else if ((b & 0xE0) == 0xC0) {
            cpB = b & 0x1F;
            if (byteIdx + 1 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 1] & 0x3F);
            byteIdx += 2;
        } else if ((b & 0xF0) == 0xE0) {
            cpB = b & 0x0F;
            if (byteIdx + 1 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 1] & 0x3F);
            if (byteIdx + 2 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 2] & 0x3F);
            byteIdx += 3;
        } else if ((b & 0xF8) == 0xF0) {
            cpB = b & 0x07;
            if (byteIdx + 1 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 1] & 0x3F);
            if (byteIdx + 2 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 2] & 0x3F);
            if (byteIdx + 3 < lenByteB) cpB = (cpB << 6) | (bytes[byteIdx + 3] & 0x3F);
            byteIdx += 4;
        } else {
            byteIdx += 1; continue; 
        }
        
        curr[0] = static_cast<int>(i);
        int minRowCost = curr[0];

        for (size_t j = 1; j <= lenA; ++j) {
            int cost = (cpA[j - 1] == cpB) ? 0 : 1;
            curr[j] = std::min({ prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost });
            if (curr[j] < minRowCost) minRowCost = curr[j];
        }
        
        if (minRowCost > maxDist) return maxDist + 1;
        for (size_t j = 0; j <= lenA; ++j) prev[j] = curr[j];
        ++i;
    }
    
    size_t lenB = i - 1;
    if (std::abs(static_cast<int>(lenA) - static_cast<int>(lenB)) > maxDist) return maxDist + 1; 

    return prev[lenA];
}

} // namespace bijoy::spelling
