#include "core/phonetic_parser.h"
#include "core/phonetic_engine.h"
#include "core/layout.h"
#include "core/startup_options.h"
#include "core/platform_types.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#define VK_BACK       0x08
#define VK_TAB        0x09
#define VK_RETURN     0x0D
#define VK_SPACE      0x20
#define VK_LEFT       0x25
#define VK_F11        0x7A
#define VK_F12        0x7B
#define VK_OEM_1      0xBA
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_3      0xC0
#endif

int main() {
    bijoy::core::PhoneticParser parser;

    // Helper tests
    assert(parser.IsVowel('A') == true);
    assert(parser.IsVowel('b') == false);
    assert(parser.IsConsonant('B') == true);
    assert(parser.IsConsonant('e') == false);
    assert(parser.FixString("ODEr AMAr") == "ODer amar");
    assert(parser.IsNumber('1') == true);

    struct TestCase {
        std::string input;
        std::wstring expected;
    };

    const std::vector<TestCase> testCases = {
        { "bhl", L"\u09ad\u09cd\u09b2" },
        { "bj", L"\u09ac\u09cd\u099c" },
        { "bd", L"\u09ac\u09cd\u09a6" },
        { "bb", L"\u09ac\u09cd\u09ac" },
        { "bl", L"\u09ac\u09cd\u09b2" },
        { "bh", L"\u09ad" },
        { "vl", L"\u09ad\u09cd\u09b2" },
        { "b", L"\u09ac" },
        { "v", L"\u09ad" },
        { "cNG", L"\u099a\u09cd\u099e" },
        { "cch", L"\u099a\u09cd\u099b" },
        { "cc", L"\u099a\u09cd\u099a" },
        { "ch", L"\u099b" },
        { "c", L"\u099a" },
        { "dhn", L"\u09a7\u09cd\u09a8" },
        { "dhm", L"\u09a7\u09cd\u09ae" },
        { "dgh", L"\u09a6\u09cd\u0998" },
        { "ddh", L"\u09a6\u09cd\u09a7" },
        { "dbh", L"\u09a6\u09cd\u09ad" },
        { "dv", L"\u09a6\u09cd\u09ad" },
        { "dm", L"\u09a6\u09cd\u09ae" },
        { "DD", L"\u09a1\u09cd\u09a1" },
        { "Dh", L"\u09a2" },
        { "dh", L"\u09a7" },
        { "dg", L"\u09a6\u09cd\u0997" },
        { "dd", L"\u09a6\u09cd\u09a6" },
        { "D", L"\u09a1" },
        { "d", L"\u09a6" },
        { "...", L"..." },
        { ".`", L"." },
        { "..", L"\u0964\u0964" },
        { ".", L"\u0964" },
        { "ghn", L"\u0998\u09cd\u09a8" },
        { "Ghn", L"\u0998\u09cd\u09a8" },
        { "gdh", L"\u0997\u09cd\u09a7" },
        { "gN", L"\u0997\u09cd\u09a3" },
        { "GN", L"\u0997\u09cd\u09a3" },
        { "gn", L"\u0997\u09cd\u09a8" },
        { "gm", L"\u0997\u09cd\u09ae" },
        { "Gm", L"\u0997\u09cd\u09ae" },
        { "gl", L"\u0997\u09cd\u09b2" },
        { "Gl", L"\u0997\u09cd\u09b2" },
        { "gg", L"\u099c\u09cd\u099e" },
        { "GG", L"\u099c\u09cd\u099e" },
        { "Gg", L"\u099c\u09cd\u099e" },
        { "gG", L"\u099c\u09cd\u099e" },
        { "gh", L"\u0998" },
        { "Gh", L"\u0998" },
        { "g", L"\u0997" },
        { "hN", L"\u09b9\u09cd\u09a3" },
        { "hn", L"\u09b9\u09cd\u09a8" },
        { "hm", L"\u09b9\u09cd\u09ae" },
        { "hl", L"\u09b9\u09cd\u09b2" },
        { "h", L"\u09b9" },
        { "jjh", L"\u099c\u09cd\u099d" },
        { "jNG", L"\u099c\u09cd\u099e" },
        { "jh", L"\u099d" },
        { "jj", L"\u099c\u09cd\u099c" },
        { "j", L"\u099c" },
        { "J", L"\u099c" },
        { "kkhN", L"\u0995\u09cd\u09b7\u09cd\u09a3" },
        { "kShN", L"\u0995\u09cd\u09b7\u09cd\u09a3" },
        { "kkhm", L"\u0995\u09cd\u09b7\u09cd\u09ae" },
        { "kShm", L"\u0995\u09cd\u09b7\u09cd\u09ae" },
        { "kxN", L"\u0995\u09cd\u09b7\u09cd\u09a3" },
        { "kxm", L"\u0995\u09cd\u09b7\u09cd\u09ae" },
        { "kkh", L"\u0995\u09cd\u09b7" },
        { "kSh", L"\u0995\u09cd\u09b7" },
        { "ksh", L"\u0995\u09b6" },
        { "kx", L"\u0995\u09cd\u09b7" },
        { "kk", L"\u0995\u09cd\u0995" },
        { "kT", L"\u0995\u09cd\u099f" },
        { "kt", L"\u0995\u09cd\u09a4" },
        { "kl", L"\u0995\u09cd\u09b2" },
        { "ks", L"\u0995\u09cd\u09b8" },
        { "kh", L"\u0996" },
        { "k", L"\u0995" },
        { "lbh", L"\u09b2\u09cd\u09ad" },
        { "ldh", L"\u09b2\u09cd\u09a7" },
        { "lkh", L"\u09b2\u0996" },
        { "lgh", L"\u09b2\u0998" },
        { "lph", L"\u09b2\u09ab" },
        { "lk", L"\u09b2\u09cd\u0995" },
        { "lg", L"\u09b2\u09cd\u0997" },
        { "lT", L"\u09b2\u09cd\u099f" },
        { "lD", L"\u09b2\u09cd\u09a1" },
        { "lp", L"\u09b2\u09cd\u09aa" },
        { "lv", L"\u09b2\u09cd\u09ad" },
        { "lm", L"\u09b2\u09cd\u09ae" },
        { "ll", L"\u09b2\u09cd\u09b2" },
        { "lb", L"\u09b2\u09cd\u09ac" },
        { "l", L"\u09b2" },
        { "mth", L"\u09ae\u09cd\u09a5" },
        { "mph", L"\u09ae\u09cd\u09ab" },
        { "mbh", L"\u09ae\u09cd\u09ad" },
        { "mpl", L"\u09ae\u09aa\u09cd\u09b2" },
        { "mn", L"\u09ae\u09cd\u09a8" },
        { "mp", L"\u09ae\u09cd\u09aa" },
        { "mv", L"\u09ae\u09cd\u09ad" },
        { "mm", L"\u09ae\u09cd\u09ae" },
        { "ml", L"\u09ae\u09cd\u09b2" },
        { "mb", L"\u09ae\u09cd\u09ac" },
        { "mf", L"\u09ae\u09cd\u09ab" },
        { "m", L"\u09ae" },
        { "0", L"\u09e6" },
        { "1", L"\u09e7" },
        { "2", L"\u09e8" },
        { "3", L"\u09e9" },
        { "4", L"\u09ea" },
        { "5", L"\u09eb" },
        { "6", L"\u09ec" },
        { "7", L"\u09ed" },
        { "8", L"\u09ee" },
        { "9", L"\u09ef" },
        { "NgkSh", L"\u0999\u09cd\u0995\u09cd\u09b7" },
        { "Ngkkh", L"\u0999\u09cd\u0995\u09cd\u09b7" },
        { "NGch", L"\u099e\u09cd\u099b" },
        { "Nggh", L"\u0999\u09cd\u0998" },
        { "Ngkh", L"\u0999\u09cd\u0996" },
        { "NGjh", L"\u099e\u09cd\u099d" },
        { "ngOU", L"\u0999\u09cd\u0997\u09cc" },
        { "ngOI", L"\u0999\u09cd\u0997\u09c8" },
        { "Ngkx", L"\u0999\u09cd\u0995\u09cd\u09b7" },
        { "NGc", L"\u099e\u09cd\u099a" },
        { "nch", L"\u099e\u09cd\u099b" },
        { "njh", L"\u099e\u09cd\u099d" },
        { "ngh", L"\u0999\u09cd\u0998" },
        { "Ngk", L"\u0999\u09cd\u0995" },
        { "Ngx", L"\u0999\u09cd\u09b7" },
        { "Ngg", L"\u0999\u09cd\u0997" },
        { "Ngm", L"\u0999\u09cd\u09ae" },
        { "NGj", L"\u099e\u09cd\u099c" },
        { "ndh", L"\u09a8\u09cd\u09a7" },
        { "nTh", L"\u09a8\u09cd\u09a0" },
        { "NTh", L"\u09a3\u09cd\u09a0" },
        { "nth", L"\u09a8\u09cd\u09a5" },
        { "nkh", L"\u0999\u09cd\u0996" },
        { "ngo", L"\u0999\u09cd\u0997" },
        { "nga", L"\u0999\u09cd\u0997\u09be" },
        { "ngi", L"\u0999\u09cd\u0997\u09bf" },
        { "ngI", L"\u0999\u09cd\u0997\u09c0" },
        { "ngu", L"\u0999\u09cd\u0997\u09c1" },
        { "ngU", L"\u0999\u09cd\u0997\u09c2" },
        { "nge", L"\u0999\u09cd\u0997\u09c7" },
        { "ngO", L"\u0999\u09cd\u0997\u09cb" },
        { "NDh", L"\u09a3\u09cd\u09a2" },
        { "nsh", L"\u09a8\u09b6" },
        { "Ngr", L"\u0999\u09b0" },
        { "NGr", L"\u099e\u09b0" },
        { "ngr", L"\u0982\u09b0" },
        { "nj", L"\u099e\u09cd\u099c" },
        { "Ng", L"\u0999" },
        { "NG", L"\u099e" },
        { "nk", L"\u0999\u09cd\u0995" },
        { "ng", L"\u0982" },
        { "nn", L"\u09a8\u09cd\u09a8" },
        { "NN", L"\u09a3\u09cd\u09a3" },
        { "Nn", L"\u09a3\u09cd\u09a8" },
        { "nm", L"\u09a8\u09cd\u09ae" },
        { "Nm", L"\u09a3\u09cd\u09ae" },
        { "nd", L"\u09a8\u09cd\u09a6" },
        { "nT", L"\u09a8\u09cd\u099f" },
        { "NT", L"\u09a3\u09cd\u099f" },
        { "nD", L"\u09a8\u09cd\u09a1" },
        { "ND", L"\u09a3\u09cd\u09a1" },
        { "nt", L"\u09a8\u09cd\u09a4" },
        { "ns", L"\u09a8\u09cd\u09b8" },
        { "nc", L"\u099e\u09cd\u099a" },
        { "n", L"\u09a8" },
        { "N", L"\u09a3" },
        { "OI`", L"\u09c8" },
        { "OU`", L"\u09cc" },
        { "O`", L"\u09cb" },
        { "OI", L"\u0990" },
        { "kOI", L"\u0995\u09c8" },
        { " OI", L" \u0990" },
        { "(OI", L"(\u0990" },
        { ".OI", L"\u0964\u0990" },
        { "OU", L"\u0994" },
        { "kOU", L"\u0995\u09cc" },
        { " OU", L" \u0994" },
        { "-OU", L"-\u0994" },
        { ",,OU", L"\u09cd\u200c\u0994" },
        { "O", L"\u0993" },
        { "pO", L"\u09aa\u09cb" },
        { " O", L" \u0993" },
        { "iO", L"\u0987\u0993" },
        { "`O", L"\u0993" },
        { "phl", L"\u09ab\u09cd\u09b2" },
        { "pT", L"\u09aa\u09cd\u099f" },
        { "pt", L"\u09aa\u09cd\u09a4" },
        { "pn", L"\u09aa\u09cd\u09a8" },
        { "pp", L"\u09aa\u09cd\u09aa" },
        { "pl", L"\u09aa\u09cd\u09b2" },
        { "ps", L"\u09aa\u09cd\u09b8" },
        { "ph", L"\u09ab" },
        { "fl", L"\u09ab\u09cd\u09b2" },
        { "f", L"\u09ab" },
        { "p", L"\u09aa" },
        { "rri`", L"\u09c3" },
        { "rri", L"\u098b" },
        { "krri", L"\u0995\u09c3" },
        { "Irri", L"\u0988\u098b" },
        { "^rri", L"\u0981\u098b" },
        { ":rri", L"\u0983\u098b" },
        { "rZ", L"\u09b0\u200d\u09cd\u09af" },
        { "krZ", L"\u0995\u09cd\u09b0\u09cd\u09af" },
        { "rrZ", L"\u09b0\u09b0\u200d\u09cd\u09af" },
        { "yrZ", L"\u0987\u09df\u09b0\u200d\u09cd\u09af" },
        { "wrZ", L"\u0993\u09b0\u200d\u09cd\u09af" },
        { "xrZ", L"\u098f\u0995\u09cd\u09b8\u09b0\u200d\u09cd\u09af" },
        { "irZ", L"\u0987\u09b0\u200d\u09cd\u09af" },
        { "-rZ", L"-\u09b0\u200d\u09cd\u09af" },
        { "rrrZ", L"\u09b0\u09b0\u09b0\u200d\u09cd\u09af" },
        { "ry", L"\u09b0\u200d\u09cd\u09af" },
        { "qry", L"\u0995\u09cd\u09b0\u09cd\u09af" },
        { "rry", L"\u09b0\u09b0\u200d\u09cd\u09af" },
        { "yry", L"\u0987\u09df\u09b0\u200d\u09cd\u09af" },
        { "wry", L"\u0993\u09b0\u200d\u09cd\u09af" },
        { "xry", L"\u098f\u0995\u09cd\u09b8\u09b0\u200d\u09cd\u09af" },
        { "0ry", L"\u09e6\u09b0\u200d\u09cd\u09af" },
        { "rrrry", L"\u09b0\u09b0\u09b0\u09b0\u200d\u09cd\u09af" },
        { "Rry", L"\u09dc\u09cd\u09b0\u09cd\u09af" },
        { "rr", L"\u09b0\u09b0" },
        { "arr", L"\u0986\u09b0\u09b0" },
        { "arrk", L"\u0986\u09b0\u09cd\u0995" },
        { "arra", L"\u0986\u09b0\u09b0\u09be" },
        { "arr", L"\u0986\u09b0\u09b0" },
        { "arr!", L"\u0986\u09b0\u09b0!" },
        { "krr", L"\u0995\u09cd\u09b0\u09b0" },
        { "krra", L"\u0995\u09cd\u09b0\u09b0\u09be" },
        { "Rg", L"\u09dc\u09cd\u0997" },
        { "Rh", L"\u09dd" },
        { "R", L"\u09dc" },
        { "r", L"\u09b0" },
        { "or", L"\u0985\u09b0" },
        { "mr", L"\u09ae\u09cd\u09b0" },
        { "1r", L"\u09e7\u09b0" },
        { "+r", L"+\u09b0" },
        { "rr", L"\u09b0\u09b0" },
        { "yr", L"\u0987\u09df\u09b0" },
        { "wr", L"\u0993\u09b0" },
        { "xr", L"\u098f\u0995\u09cd\u09b8\u09b0" },
        { "zr", L"\u09af\u09cd\u09b0" },
        { "mri", L"\u09ae\u09cd\u09b0\u09bf" },
        { "shch", L"\u09b6\u09cd\u099b" },
        { "ShTh", L"\u09b7\u09cd\u09a0" },
        { "Shph", L"\u09b7\u09cd\u09ab" },
        { "Sch", L"\u09b6\u09cd\u099b" },
        { "skl", L"\u09b8\u09cd\u0995\u09cd\u09b2" },
        { "skh", L"\u09b8\u09cd\u0996" },
        { "sth", L"\u09b8\u09cd\u09a5" },
        { "sph", L"\u09b8\u09cd\u09ab" },
        { "shc", L"\u09b6\u09cd\u099a" },
        { "sht", L"\u09b6\u09cd\u09a4" },
        { "shn", L"\u09b6\u09cd\u09a8" },
        { "shm", L"\u09b6\u09cd\u09ae" },
        { "shl", L"\u09b6\u09cd\u09b2" },
        { "Shk", L"\u09b7\u09cd\u0995" },
        { "ShT", L"\u09b7\u09cd\u099f" },
        { "ShN", L"\u09b7\u09cd\u09a3" },
        { "Shp", L"\u09b7\u09cd\u09aa" },
        { "Shf", L"\u09b7\u09cd\u09ab" },
        { "Shm", L"\u09b7\u09cd\u09ae" },
        { "spl", L"\u09b8\u09cd\u09aa\u09cd\u09b2" },
        { "sk", L"\u09b8\u09cd\u0995" },
        { "Sc", L"\u09b6\u09cd\u099a" },
        { "sT", L"\u09b8\u09cd\u099f" },
        { "st", L"\u09b8\u09cd\u09a4" },
        { "sn", L"\u09b8\u09cd\u09a8" },
        { "sp", L"\u09b8\u09cd\u09aa" },
        { "sf", L"\u09b8\u09cd\u09ab" },
        { "sm", L"\u09b8\u09cd\u09ae" },
        { "sl", L"\u09b8\u09cd\u09b2" },
        { "sh", L"\u09b6" },
        { "Sc", L"\u09b6\u09cd\u099a" },
        { "St", L"\u09b6\u09cd\u09a4" },
        { "Sn", L"\u09b6\u09cd\u09a8" },
        { "Sm", L"\u09b6\u09cd\u09ae" },
        { "Sl", L"\u09b6\u09cd\u09b2" },
        { "Sh", L"\u09b7" },
        { "s", L"\u09b8" },
        { "S", L"\u09b6" },
        { "oo", L"\u0989" },
        { "OO", L"\u0993\u0993" },
        { "oo`", L"\u09c1" },
        { "koo", L"\u0995\u09c1" },
        { "ooo", L"\u0989\u0985" },
        { "!oo", L"!\u0989" },
        { "!ooo", L"!\u0989\u0985" },
        { "aoo", L"\u0986\u0989" },
        { "oop", L"\u0989\u09aa" },
        { "ooo`", L"\u0989" },
        { "o`", L"" },
        { "oZ", L"\u0985\u09cd\u09af" },
        { "oY", L"\u0985\u09df" },
        { "o", L"\u0985" },
        { "!o", L"!\u0985" },
        { "^o", L"\u0981\u0985" },
        { "*o", L"*\u0985" },
        { "io", L"\u0987\u0993" },
        { "yo", L"\u0987\u09df" },
        { "no", L"\u09a8" },
        { "tth", L"\u09a4\u09cd\u09a5" },
        { "t``", L"\u09ce" },
        { "`t``", L"\u09ce" },
        { "t``t``", L"\u09ce\u09ce" },
        { "t```", L"\u09ce" },
        { "TT", L"\u099f\u09cd\u099f" },
        { "Tm", L"\u099f\u09cd\u09ae" },
        { "Th", L"\u09a0" },
        { "tn", L"\u09a4\u09cd\u09a8" },
        { "tm", L"\u09a4\u09cd\u09ae" },
        { "th", L"\u09a5" },
        { "tt", L"\u09a4\u09cd\u09a4" },
        { "T", L"\u099f" },
        { "t", L"\u09a4" },
        { "aZ", L"\u0985\u09cd\u09af\u09be" },
        { "aaZ", L"\u0986\u0985\u09cd\u09af\u09be" },
        { "AZ", L"\u0985\u09cd\u09af\u09be" },
        { "a`", L"\u09be" },
        { "a``", L"\u09be" },
        { "ka`", L"\u0995\u09be" },
        { "A`", L"\u09be" },
        { "a", L"\u0986" },
        { "`a", L"\u0986" },
        { "k`a", L"\u0995\u0986" },
        { "ia", L"\u0987\u09df\u09be" },
        { "aaaa`", L"\u0986\u0986\u0986\u09be" },
        { "i`", L"\u09bf" },
        { "i", L"\u0987" },
        { "`i", L"\u0987" },
        { "hi", L"\u09b9\u09bf" },
        { "ih", L"\u0987\u09b9" },
        { "i`h", L"\u09bf\u09b9" },
        { "I`", L"\u09c0" },
        { "I", L"\u0988" },
        { "cI", L"\u099a\u09c0" },
        { "Ix", L"\u0988\u0995\u09cd\u09b8" },
        { "II", L"\u0988\u0988" },
        { "0I", L"\u09e6\u0988" },
        { "oI", L"\u0985\u0988" },
        { "u`", L"\u09c1" },
        { "u", L"\u0989" },
        { "ku", L"\u0995\u09c1" },
        { "uk", L"\u0989\u0995" },
        { "uu", L"\u0989\u0989" },
        { "iu", L"\u0987\u0989" },
        { "&u", L"&\u0989" },
        { "u&", L"\u0989&" },
        { "U`", L"\u09c2" },
        { "U", L"\u098a" },
        { "yU", L"\u0987\u09df\u09c2" },
        { "Uy", L"\u098a\u09df" },
        { "^U", L"\u0981\u098a" },
        { "U^", L"\u098a\u0981" },
        { "EE", L"\u0988" },
        { "ee", L"\u0988" },
        { "Ee", L"\u0988" },
        { "eE", L"\u0988" },
        { "ee`", L"\u09c0" },
        { "kee", L"\u0995\u09c0" },
        { "eek", L"\u0988\u0995" },
        { "0ee", L"\u09e6\u0988" },
        { "ee8", L"\u0988\u09ee" },
        { "(ee)", L"(\u0988)" },
        { "e`", L"\u09c7" },
        { "e", L"\u098f" },
        { "ke", L"\u0995\u09c7" },
        { "we", L"\u0993\u09df\u09c7" },
        { "#e#", L"#\u098f#" },
        { "`e`", L"\u09c7" },
        { "z", L"\u09af" },
        { "Z", L"\u09cd\u09af" },
        { "rZ", L"\u09b0\u200d\u09cd\u09af" },
        { "kZS", L"\u0995\u09cd\u09af\u09b6" },
        { "y", L"\u0987\u09df" },
        { "oy", L"\u0985\u09df" },
        { "ky", L"\u0995\u09cd\u09af" },
        { "ya", L"\u0987\u09df\u09be" },
        { "yaa", L"\u0987\u09df\u09be\u0986" },
        { "Y", L"\u09df" },
        { "YY", L"\u09df\u09df" },
        { "iY", L"\u0987\u09df" },
        { "kY", L"\u0995\u09df" },
        { "q", L"\u0995" },
        { "Q", L"\u0995" },
        { "w", L"\u0993" },
        { "wa", L"\u0993\u09df\u09be" },
        { "-wa-", L"-\u0993\u09df\u09be-" },
        { "woo", L"\u0993\u09df\u09c1" },
        { "wre", L"\u0993\u09b0\u09c7" },
        { "kw", L"\u0995\u09cd\u09ac" },
        { "x", L"\u098f\u0995\u09cd\u09b8" },
        { "ex", L"\u098f\u0995\u09cd\u09b8" },
        { "bx", L"\u09ac\u0995\u09cd\u09b8" },
        { ":`", L":" },
        { ":", L"\u0983" },
        { "^`", L"^" },
        { "^", L"\u0981" },
        { "k^", L"\u0995\u0981" },
        { "k^i", L"\u0995\u0981\u0987" },
        { "ki^", L"\u0995\u09bf\u0981" },
        { ",,", L"\u09cd\u200c" },
        { ",,,", L"\u09cd\u200c," },
        { ",,`,", L"\u09cd\u200c," },
        { "`,,", L"\u09cd\u200c" },
        { ",`,", L",," },
        { "$", L"\u09f3" },
        { "`", L"" },
        { "bdh", L"\u09ac\u09cd\u09a7" },
        { "ami banglay gan gai", L"\u0986\u09ae\u09bf \u09ac\u09be\u0982\u09b2\u09be\u09df \u0997\u09be\u09a8 \u0997\u09be\u0987" },
        { "amader valObasa hoye gel ghas, kheye gel goru ar diye gelo ba^sh", L"\u0986\u09ae\u09be\u09a6\u09c7\u09b0 \u09ad\u09be\u09b2\u09cb\u09ac\u09be\u09b8\u09be \u09b9\u09df\u09c7 \u0997\u09c7\u09b2 \u0998\u09be\u09b8, \u0996\u09c7\u09df\u09c7 \u0997\u09c7\u09b2 \u0997\u09b0\u09c1 \u0986\u09b0 \u09a6\u09bf\u09df\u09c7 \u0997\u09c7\u09b2 \u09ac\u09be\u0981\u09b6" },
    };

    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& tc = testCases[i];
        std::wstring actual = parser.Convert(tc.input);
        if (actual == tc.expected) {
            passed++;
        } else {
            failed++;
            std::wcout << L"FAILED Test #" << i << L": input=\"" << tc.input.c_str() 
                       << L"\", expected len=" << tc.expected.length() 
                       << L", actual len=" << actual.length() << std::endl;
        }
    }

    std::cout << "PhoneticParser Tests: " << passed << " passed, " << failed << " failed." << std::endl;

    // --- PhoneticEngine Diff and Key Mapping Tests ---
    {
        // 1. ComputeDiff tests
        auto diff1 = bijoy::core::PhoneticEngine::ComputeDiff(L"\u0995", L"\u0996"); // "ক" -> "খ"
        assert(diff1.commonPrefixLength == 0);
        assert(diff1.backspaceCount == 1);
        assert(diff1.toSend == L"\u0996");

        auto diff2 = bijoy::core::PhoneticEngine::ComputeDiff(L"\u0986", L"\u0986\u09ae"); // "আ" -> "আম"
        assert(diff2.commonPrefixLength == 1);
        assert(diff2.backspaceCount == 0);
        assert(diff2.toSend == L"\u09ae");

        auto diff3 = bijoy::core::PhoneticEngine::ComputeDiff(L"\u0986\u09ae", L"\u0986\u09ae\u09bf"); // "আম" -> "আমি"
        assert(diff3.commonPrefixLength == 2);
        assert(diff3.backspaceCount == 0);
        assert(diff3.toSend == L"\u09bf");

        auto diff4 = bijoy::core::PhoneticEngine::ComputeDiff(L"\u0986\u09ae\u09bf", L"\u0986\u09ae"); // "আমি" -> "আম"
        assert(diff4.commonPrefixLength == 2);
        assert(diff4.backspaceCount == 1);
        assert(diff4.toSend.empty());

        auto diff5 = bijoy::core::PhoneticEngine::ComputeDiff(L"", L"\u0995"); // "" -> "ক"
        assert(diff5.commonPrefixLength == 0);
        assert(diff5.backspaceCount == 0);
        assert(diff5.toSend == L"\u0995");

        auto diff6 = bijoy::core::PhoneticEngine::ComputeDiff(L"\u0995", L""); // "ক" -> ""
        assert(diff6.commonPrefixLength == 0);
        assert(diff6.backspaceCount == 1);
        assert(diff6.toSend.empty());

        // 2. MapVirtualKeyToChar tests
        bijoy::core::PhoneticEngine engine;
        assert(engine.MapVirtualKeyToChar('A', false) == 'a');
        assert(engine.MapVirtualKeyToChar('A', true) == 'A');
        assert(engine.MapVirtualKeyToChar('Z', false) == 'z');
        assert(engine.MapVirtualKeyToChar('Z', true) == 'Z');
        assert(engine.MapVirtualKeyToChar('1', false) == '1');
        assert(engine.MapVirtualKeyToChar('1', true) == '!');
        assert(engine.MapVirtualKeyToChar('4', true) == '$');
        assert(engine.MapVirtualKeyToChar('6', true) == '^');
        assert(engine.MapVirtualKeyToChar(VK_OEM_PERIOD, false) == '.');
        assert(engine.MapVirtualKeyToChar(VK_OEM_3, false) == '`');
        assert(engine.MapVirtualKeyToChar(VK_OEM_1, true) == ':');
        assert(engine.MapVirtualKeyToChar(VK_OEM_1, false) == ';');

#if defined(_WIN32)
        bijoy::core::NativeWindowHandle dummyWnd = reinterpret_cast<bijoy::core::NativeWindowHandle>(static_cast<uintptr_t>(100));
#else
        bijoy::core::NativeWindowHandle dummyWnd = 100;
#endif
        // Ctrl+C passes through
        assert(engine.ProcessKey('C', true, false, true, false, false, dummyWnd) == false);
        // Alt+Tab passes through
        assert(engine.ProcessKey(VK_TAB, true, true, false, true, false, dummyWnd) == false);
        // Backspace on empty buffer passes through
        assert(engine.ProcessKey(VK_BACK, true, false, false, false, false, dummyWnd) == false);
        // Space passes through
        assert(engine.ProcessKey(VK_SPACE, true, false, false, false, false, dummyWnd) == false);
        // Enter passes through
        assert(engine.ProcessKey(VK_RETURN, true, false, false, false, false, dummyWnd) == false);
        // Arrow key passes through
        assert(engine.ProcessKey(VK_LEFT, true, false, false, false, false, dummyWnd) == false);

        std::cout << "PhoneticEngine Tests: All diff, mapping, and state tests passed successfully!" << std::endl;
    }

    // 4. Test loading 02 Phonetic.xml
    {
        bijoy::core::Layout phoneticLayout;
        bool loaded = phoneticLayout.loadFromFile(L"build/data/Layouts/02 Phonetic.xml");
        assert(loaded);
        assert(phoneticLayout.isPhonetic);
        assert(phoneticLayout.name == L"Phonetic");
        assert(phoneticLayout.shortcut.keyCode == 123);
        assert(!phoneticLayout.shortcut.ctrl);
        assert(!phoneticLayout.shortcut.alt);
        assert(!phoneticLayout.shortcut.shift);
        std::cout << "Phonetic XML Layout Test: 02 Phonetic.xml loaded and verified successfully!" << std::endl;
    }

    // 5. Test StartupOptions and Custom Layout Shortcuts persistence
    {
        bijoy::core::StartupOptions opts;
        opts.autoStart = false;
        opts.overlayOpacity = 85;
        opts.alwaysOnTop = true;
        opts.snapToTop = true;
        opts.showSplash = true;
        opts.phoneticToggleKey = VK_F11;
        opts.phoneticToggleCtrl = true;
        opts.phoneticToggleAlt = false;
        opts.phoneticToggleShift = true;
        opts.defaultLayout = 1;

        bool saved = bijoy::core::SaveStartupOptions(opts);
        assert(saved);

        bijoy::core::StartupOptions loaded = bijoy::core::LoadStartupOptions();
        assert(loaded.overlayOpacity == 85);
        assert(loaded.alwaysOnTop == true);
        assert(loaded.snapToTop == true);
        assert(loaded.showSplash == true);
        assert(loaded.phoneticToggleKey == VK_F11);
        assert(loaded.phoneticToggleCtrl == true);
        assert(loaded.phoneticToggleAlt == false);
        assert(loaded.phoneticToggleShift == true);
        assert(loaded.defaultLayout == 1);

        // Test custom layout shortcut persistence
        bijoy::core::LayoutShortcut sc;
        sc.keyCode = 'V';
        sc.ctrl = true;
        sc.alt = true;
        sc.shift = false;
        bool scSaved = bijoy::core::SaveCustomLayoutShortcut(L"TestLayout", sc);
        assert(scSaved);

        bijoy::core::LayoutShortcut scLoaded;
        bool scFound = bijoy::core::LoadCustomLayoutShortcut(L"TestLayout", scLoaded);
        assert(scFound);
        assert(scLoaded.keyCode == 'V');
        assert(scLoaded.ctrl == true);
        assert(scLoaded.alt == true);
        assert(scLoaded.shift == false);

        bool scDeleted = bijoy::core::DeleteCustomLayoutShortcut(L"TestLayout");
        assert(scDeleted);
        assert(!bijoy::core::LoadCustomLayoutShortcut(L"TestLayout", scLoaded));

        // Restore default options
        opts.overlayOpacity = 100;
        opts.phoneticToggleKey = VK_F12;
        opts.phoneticToggleCtrl = false;
        opts.phoneticToggleShift = false;
        bijoy::core::SaveStartupOptions(opts);

        std::cout << "StartupOptions & Custom Shortcuts Test: All persistence tests passed successfully!" << std::endl;
    }

    return failed == 0 ? 0 : 1;
}
