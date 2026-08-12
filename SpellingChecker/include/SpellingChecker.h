#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <mutex>

namespace bijoy::spelling {

// -------------------------------------------------------------------------
// SpellChecker
// Loads a Bangla word dictionary and provides fast O(log N) word lookup
// with highly optimized Levenshtein-distance suggestions for misspelled words.
//
// ZERO RAM POTATO PC MODE:
// Designed for a 7 Billion+ word dictionary. File is NEVER loaded to RAM.
// We binary search directly on the disk using file seek offsets.
// Suggestions read a tiny ~100KB block around the target word on disk.
// -------------------------------------------------------------------------
class SpellChecker {
public:
    SpellChecker();
    ~SpellChecker();

    // Open dictionary from a text file (one word per line, UTF-8, ALPHABETICALLY SORTED)
    // Returns 0 on success, -1 on failure
    int LoadDictionary(const std::string& filePath);

    // Check if a word exists in the dictionary (Uses binary search on DISK)
    bool Check(const std::string& word);

    // Get spelling suggestions for a misspelled word
    std::vector<std::string> Suggest(const std::string& word, int maxResults = 5);

    // Returns a dummy large size or just the file size in bytes
    size_t GetDictionarySize() const;

private:
    std::ifstream m_file;
    uintmax_t m_fileSize;
    mutable std::mutex m_mutex;

    // Helper to read the word immediately following the given byte offset
    // Also returns the exact byte start offset of the word it read
    bool ReadNextWordFromOffset(uintmax_t offset, std::string& outWord, uintmax_t& outStartOffset);

    // Helper that returns the closest byte offset in the dictionary where `word` should be
    uintmax_t FindInsertionOffset(const std::string& word);

    // Compute Levenshtein edit distance with early abort.
    static int LevenshteinDistanceEarlyAbort(
        const std::vector<uint32_t>& cpA, 
        const std::string& bUtf8,
        int maxDist);

    static std::vector<uint32_t> Utf8ToCodepoints(const std::string& str);
};

} // namespace bijoy::spelling
