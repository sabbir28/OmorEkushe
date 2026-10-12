#pragma once

#include "core/phonetic_data.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace bijoy::core {

class PhoneticParser {
public:
    PhoneticParser();
    explicit PhoneticParser(const std::vector<PhoneticPattern>& patterns);

    std::wstring Convert(const std::string& input) const;
    std::string FixString(const std::string& input) const;

    bool IsVowel(char c) const;
    bool IsConsonant(char c) const;
    bool IsNumber(char c) const;
    bool IsPunctuation(char c) const;
    bool IsCaseSensitive(char c) const;

private:
    void Initialize(const std::vector<PhoneticPattern>& patterns);
    bool EvaluateMatch(const PhoneticMatchCondition& match, const std::string& fixed, int start, int end) const;

    std::vector<PhoneticPattern> m_patterns;
    std::unordered_map<std::string, size_t> m_patternIndex;
    int m_maxPatternLen = 1;
};

} // namespace bijoy::core

