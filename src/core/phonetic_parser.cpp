#include "core/phonetic_parser.h"
#include "core/omorekushe_fast.h"

#include <cctype>
#include <algorithm>
#include <array>
#include <utility>

namespace bijoy::core {

namespace {

constexpr uint8_t InitCaseMap(size_t i) {
    uint8_t b = static_cast<uint8_t>(i);
    uint8_t lower = (b >= 'A' && b <= 'Z') ? static_cast<uint8_t>(b + 32) : b;
    bool isCaseSens = (lower == 'o' || lower == 'i' || lower == 'u' || lower == 'd' ||
                       lower == 'g' || lower == 'j' || lower == 'n' || lower == 'r' ||
                       lower == 's' || lower == 't' || lower == 'y' || lower == 'z');
    return isCaseSens ? b : lower;
}

template <size_t... Is>
constexpr auto MakeCaseTable(std::index_sequence<Is...>) {
    return std::array<uint8_t, sizeof...(Is)>{InitCaseMap(Is)...};
}

constexpr auto kCaseTable = MakeCaseTable(std::make_index_sequence<256>{});

} // namespace

PhoneticParser::PhoneticParser() {
    Initialize(GetBuiltinPhoneticPatterns());
}

PhoneticParser::PhoneticParser(const std::vector<PhoneticPattern>& patterns) {
    Initialize(patterns);
}

void PhoneticParser::Initialize(const std::vector<PhoneticPattern>& patterns) {
    m_patterns = patterns;
    m_patternIndex.clear();
    m_maxPatternLen = 1;

    for (size_t i = 0; i < m_patterns.size(); ++i) {
        m_patternIndex[m_patterns[i].find] = i;
        if (static_cast<int>(m_patterns[i].find.length()) > m_maxPatternLen) {
            m_maxPatternLen = static_cast<int>(m_patterns[i].find.length());
        }
    }
}

bool PhoneticParser::IsVowel(char c) const {
    const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower == 'a' || lower == 'e' || lower == 'i' || lower == 'o' || lower == 'u';
}

bool PhoneticParser::IsConsonant(char c) const {
    const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return (lower >= 'a' && lower <= 'z') && !IsVowel(lower);
}

bool PhoneticParser::IsNumber(char c) const {
    return c >= '0' && c <= '9';
}

bool PhoneticParser::IsPunctuation(char c) const {
    return !IsVowel(c) && !IsConsonant(c);
}

bool PhoneticParser::IsCaseSensitive(char c) const {
    const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower == 'o' || lower == 'i' || lower == 'u' || lower == 'd' ||
           lower == 'g' || lower == 'j' || lower == 'n' || lower == 'r' ||
           lower == 's' || lower == 't' || lower == 'y' || lower == 'z';
}

std::string PhoneticParser::FixString(const std::string& input) const {
    std::string result(input.length(), '\0');
#if defined(OMOREKUSHE_ENABLE_RUST)
    omorekushe_fast_fix_string(
        reinterpret_cast<const uint8_t*>(input.data()), input.length(),
        reinterpret_cast<uint8_t*>(&result[0]), result.length()
    );
#else
    for (size_t i = 0; i < input.length(); ++i) {
        result[i] = static_cast<char>(kCaseTable[static_cast<unsigned char>(input[i])]);
    }
#endif
    return result;
}

bool PhoneticParser::EvaluateMatch(const PhoneticMatchCondition& match, const std::string& fixed, int start, int end) const {
    const int len = static_cast<int>(fixed.length());
    const int chk = (match.type == PhoneticMatchType::Suffix) ? end : (start - 1);

    bool cond = false;
    switch (match.scope) {
        case PhoneticScope::Punctuation:
            if (match.type == PhoneticMatchType::Prefix) {
                cond = (chk < 0) || (chk >= 0 && chk < len && IsPunctuation(fixed[chk]));
            } else {
                cond = (chk >= len) || (chk >= 0 && chk < len && IsPunctuation(fixed[chk]));
            }
            break;
        case PhoneticScope::Vowel:
            cond = (chk >= 0 && chk < len && IsVowel(fixed[chk]));
            break;
        case PhoneticScope::Consonant:
            cond = (chk >= 0 && chk < len && IsConsonant(fixed[chk]));
            break;
        case PhoneticScope::Number:
            cond = (chk >= 0 && chk < len && IsNumber(fixed[chk]));
            break;
        case PhoneticScope::Exact: {
            const int vlen = static_cast<int>(match.value.length());
            int s = 0, e = 0;
            if (match.type == PhoneticMatchType::Suffix) {
                s = end;
                e = end + vlen;
            } else {
                s = start - vlen;
                e = start;
            }
            cond = (s >= 0 && e <= len && fixed.substr(s, vlen) == match.value);
            break;
        }
    }

    return match.negative ? !cond : cond;
}

std::wstring PhoneticParser::Convert(const std::string& input) const {
    const std::string fixed = FixString(input);
    const int len = static_cast<int>(fixed.length());
    std::wstring output;
    output.reserve(len * 2);

    int cur = 0;
    while (cur < len) {
        const int start = cur;
        int end = 0;
        bool matched = false;

        for (int chunk_len = m_maxPatternLen; chunk_len >= 1; --chunk_len) {
            end = start + chunk_len;
            if (end <= len) {
                const std::string chunk = fixed.substr(start, chunk_len);
                auto it = m_patternIndex.find(chunk);
                if (it != m_patternIndex.end()) {
                    const PhoneticPattern& pattern = m_patterns[it->second];
                    if (!pattern.rules.empty()) {
                        for (const auto& rule : pattern.rules) {
                            bool replace = true;
                            for (const auto& match : rule.matches) {
                                if (!EvaluateMatch(match, fixed, start, end)) {
                                    replace = false;
                                    break;
                                }
                            }
                            if (replace) {
                                output += rule.replace;
                                cur = end - 1;
                                matched = true;
                                break;
                            }
                        }
                    }

                    if (matched) {
                        break;
                    }

                    // Default replacement
                    output += pattern.replace;
                    cur = end - 1;
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            output += static_cast<wchar_t>(static_cast<unsigned char>(fixed[cur]));
        }
        ++cur;
    }

    return output;
}

} // namespace bijoy::core

