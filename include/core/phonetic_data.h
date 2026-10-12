#pragma once

#include <string>
#include <vector>

namespace bijoy::core {

enum class PhoneticMatchType {
    Prefix,
    Suffix
};

enum class PhoneticScope {
    Punctuation,
    Vowel,
    Consonant,
    Number,
    Exact
};

struct PhoneticMatchCondition {
    PhoneticMatchType type;
    PhoneticScope scope;
    bool negative = false;
    std::string value;
};

struct PhoneticRule {
    std::vector<PhoneticMatchCondition> matches;
    std::wstring replace;
};

struct PhoneticPattern {
    std::string find;
    std::wstring replace;
    std::vector<PhoneticRule> rules;
};

const std::vector<PhoneticPattern>& GetBuiltinPhoneticPatterns();

} // namespace bijoy::core
