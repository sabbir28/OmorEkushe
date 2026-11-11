#include "SpellingChecker.h"
#include "core/omorekushe_fast.h"

#include <algorithm>
#include <queue>
#include <filesystem>
#include <iostream>
#include <cstring>

#if defined(_M_X64) || defined(__x86_64__)
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#endif

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

    m_file.clear();
    m_file.seekg(offset);
    char buffer[512];
    
    // Fast buffered scanning replacing 1-byte read system calls
    if (offset > 0) {
        bool foundNewline = false;
        while (!foundNewline && m_file.read(buffer, sizeof(buffer))) {
            const std::streamsize bytesRead = m_file.gcount();
            const char* nl = static_cast<const char*>(std::memchr(buffer, '\n', static_cast<size_t>(bytesRead)));
            if (nl) {
                const uintmax_t posAfterNl = offset + (nl - buffer) + 1;
                m_file.seekg(posAfterNl);
                foundNewline = true;
                break;
            }
            offset += bytesRead;
        }
        if (!foundNewline) {
            const std::streamsize bytesRead = m_file.gcount();
            if (bytesRead > 0) {
                const char* nl = static_cast<const char*>(std::memchr(buffer, '\n', static_cast<size_t>(bytesRead)));
                if (nl) {
                    const uintmax_t posAfterNl = offset + (nl - buffer) + 1;
                    m_file.seekg(posAfterNl);
                    foundNewline = true;
                }
            }
        }
        if (!foundNewline) return false;
    }

    outStartOffset = m_file.tellg();
    outWord.clear();

    // Fast buffered word extraction
    while (m_file.read(buffer, sizeof(buffer)) || m_file.gcount() > 0) {
        const std::streamsize bytesRead = m_file.gcount();
        bool foundEnd = false;
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            char c = buffer[i];
            if (c == '\r') continue;
            if (c == '\n') {
                foundEnd = true;
                m_file.seekg(outStartOffset + outWord.size() + i + 1);
                break;
            }
            outWord.push_back(c);
        }
        if (foundEnd || bytesRead == 0) break;
    }

    if (outWord.empty() && !m_file.eof() && outStartOffset < m_fileSize) {
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
    // AUDIT FIX: Clear stream flags prior to seeking to avoid silent seek failure if stream touched EOF
    m_file.clear();
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
    if (lenA >= 127) {
        return maxDist + 1;
    }

    uint32_t cpB[128];
    size_t lenB = 0;

#if defined(OMOREKUSHE_ENABLE_RUST)
    lenB = omorekushe_fast_utf8_to_codepoints(
        reinterpret_cast<const uint8_t*>(bUtf8.data()), bUtf8.size(),
        cpB, 128
    );
#else
    auto decodedB = Utf8ToCodepoints(bUtf8);
    lenB = std::min(decodedB.size(), static_cast<size_t>(127));
    std::memcpy(cpB, decodedB.data(), lenB * sizeof(uint32_t));
#endif

    if (std::abs(static_cast<int>(lenA) - static_cast<int>(lenB)) > maxDist) {
        return maxDist + 1;
    }

#if defined(OMOREKUSHE_ENABLE_RUST)
    return static_cast<int>(omorekushe_fast_levenshtein(
        cpA.data(), lenA,
        cpB, lenB,
        static_cast<uint32_t>(maxDist)
    ));
#else
    // Hardware-accelerated Myers 64-bit Bit-Parallel algorithm with assembly POPCNT
    if (lenA <= 64 && lenB <= 64) {
        uint64_t vp = ~0ULL;
        uint64_t vn = 0ULL;
        uint32_t currDist = static_cast<uint32_t>(lenA);

        for (size_t k = 0; k < lenB; ++k) {
            const uint32_t chB = cpB[k];
            uint64_t pm = 0;
            for (size_t i = 0; i < lenA; ++i) {
                if (cpA[i] == chB) pm |= (1ULL << i);
            }

            uint64_t x = pm | vn;
            uint64_t d0 = ((vp + (x & vp)) ^ vp) | x;
            uint64_t hn = vp & d0;
            uint64_t hp = vn | ~(vp | d0);
            x = (hp << 1) | 1ULL;
            vn = x & d0;
            vp = (hn << 1) | ~(x | d0);

            uint64_t maskLast = 1ULL << (lenA - 1);
            if ((hp & maskLast) != 0) {
                currDist++;
            } else if ((hn & maskLast) != 0) {
                if (currDist > 0) currDist--;
            }
        }
        return static_cast<int>(currDist);
    }

    int prev[128], curr[128];
    for (size_t j = 0; j <= lenA; ++j) prev[j] = static_cast<int>(j);

    for (size_t i = 1; i <= lenB; ++i) {
        curr[0] = static_cast<int>(i);
        int minRowCost = curr[0];

        for (size_t j = 1; j <= lenA; ++j) {
            int cost = (cpA[j - 1] == cpB[i - 1]) ? 0 : 1;
            curr[j] = std::min({ prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost });
            if (curr[j] < minRowCost) minRowCost = curr[j];
        }

        if (minRowCost > maxDist) return maxDist + 1;
        for (size_t j = 0; j <= lenA; ++j) prev[j] = curr[j];
    }

    return prev[lenA];
#endif
}

} // namespace bijoy::spelling
