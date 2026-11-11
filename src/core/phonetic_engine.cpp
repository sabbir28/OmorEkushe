#include "core/phonetic_engine.h"
#include "core/platform_input.h"
#include "core/omorekushe_fast.h"

#include <vector>
#include <algorithm>

#if defined(_M_X64) || defined(__x86_64__)
#include <immintrin.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace bijoy::core {

PhoneticEngine g_phoneticEngine;

PhoneticEngine::PhoneticEngine()
    : m_parser() {
}

PhoneticEngine::PhoneticEngine(const PhoneticParser& parser)
    : m_parser(parser) {
}

void PhoneticEngine::Reset() {
    m_rawBuffer.clear();
    m_lastOutput.clear();
}

PhoneticEngine::DiffResult PhoneticEngine::ComputeDiff(const std::wstring& oldOutput, const std::wstring& newOutput) {
    DiffResult result;
    size_t common = 0;

#if defined(OMOREKUSHE_ENABLE_RUST)
    size_t backspaces = 0;
    omorekushe_fast_compute_diff(
        reinterpret_cast<const uint16_t*>(oldOutput.data()), oldOutput.length(),
        reinterpret_cast<const uint16_t*>(newOutput.data()), newOutput.length(),
        &common, &backspaces
    );
    result.commonPrefixLength = common;
    result.backspaceCount = backspaces;
    result.toSend = newOutput.substr(common);
    return result;
#else
    const size_t minLen = std::min(oldOutput.length(), newOutput.length());

#if defined(_M_X64) || defined(__x86_64__)
    // Hardware-accelerated 128-bit vectorization (SSE2/SSE4.2/AVX) with inline assembly bit-scan
    const wchar_t* p1 = oldOutput.data();
    const wchar_t* p2 = newOutput.data();
    while (common + 8 <= minLen) {
        __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p1 + common));
        __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p2 + common));
        __m128i eq = _mm_cmpeq_epi16(v1, v2);
        int mask = _mm_movemask_epi8(eq);
        if (mask != 0xFFFF) {
            unsigned long diffByte = 0;
            unsigned int notMask = static_cast<unsigned int>(~mask) & 0xFFFF;
#if defined(_MSC_VER)
            _BitScanForward(&diffByte, notMask);
#else
            diffByte = __builtin_ctz(notMask);
#endif
            common += (diffByte / 2);
            result.commonPrefixLength = common;
            result.backspaceCount = oldOutput.length() - common;
            result.toSend = newOutput.substr(common);
            return result;
        }
        common += 8;
    }
#endif

    while (common < minLen && oldOutput[common] == newOutput[common]) {
        ++common;
    }

    result.commonPrefixLength = common;
    result.backspaceCount = oldOutput.length() - common;
    result.toSend = newOutput.substr(common);
    return result;
#endif
}

void PhoneticEngine::EmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend) {
    PlatformEmitBackspacesAndChars(backspaces, toSend);
}

void PhoneticEngine::ApplyDiff(const std::wstring& newOutput) {
    DiffResult diff = ComputeDiff(m_lastOutput, newOutput);
    EmitBackspacesAndChars(diff.backspaceCount, diff.toSend);
    m_lastOutput = newOutput;
}

char PhoneticEngine::MapVirtualKeyToChar(uint32_t vkCode, bool shift) const {
    if (vkCode >= 'A' && vkCode <= 'Z') {
#if defined(_WIN32)
        const bool caps = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
        const bool caps = false;
#endif
        const bool isUpper = shift ^ caps;
        return isUpper ? static_cast<char>(vkCode) : static_cast<char>(vkCode - 'A' + 'a');
    }

    if (vkCode >= '0' && vkCode <= '9') {
        if (shift) {
            static const char kShiftDigits[] = ")!@#$%^&*(";
            return kShiftDigits[vkCode - '0'];
        }
        return static_cast<char>(vkCode);
    }

#if defined(_WIN32)
    switch (vkCode) {
        case VK_OEM_1:      return shift ? ':' : ';';
        case VK_OEM_PLUS:   return shift ? '+' : '=';
        case VK_OEM_COMMA:  return shift ? '<' : ',';
        case VK_OEM_MINUS:  return shift ? '_' : '-';
        case VK_OEM_PERIOD: return shift ? '>' : '.';
        case VK_OEM_2:      return shift ? '?' : '/';
        case VK_OEM_3:      return shift ? '~' : '`';
        case VK_OEM_4:      return shift ? '{' : '[';
        case VK_OEM_5:      return shift ? '|' : '\\';
        case VK_OEM_6:      return shift ? '}' : ']';
        case VK_OEM_7:      return shift ? '"' : '\'';
        default: break;
    }
#else
    // Standard ASCII mappings
    if (vkCode >= 0x20 && vkCode <= 0x7E) {
        return static_cast<char>(vkCode);
    }
#endif

    return 0;
}

bool PhoneticEngine::TryExpandMacro() {
#if defined(OMOREKUSHE_ENABLE_RUST)
    uint16_t outBuf[256];
    size_t len = omorekushe_fast_expand_macro(
        m_rawBuffer.data(), m_rawBuffer.length(),
        outBuf, 256
    );
    if (len > 0) {
        std::wstring expanded(reinterpret_cast<const wchar_t*>(outBuf), len);
        ApplyDiff(expanded);
        return true;
    }
#endif
    return false;
}

std::vector<std::wstring> PhoneticEngine::GetSuggestions(size_t maxSuggestions) const {
    std::vector<std::wstring> results;
    if (m_lastOutput.empty() || maxSuggestions == 0) {
        return results;
    }

#if defined(OMOREKUSHE_ENABLE_RUST)
    uint16_t buf[512];
    size_t offsets[16];
    const size_t limit = std::min(maxSuggestions, static_cast<size_t>(16));

    size_t count = omorekushe_fast_predict_candidates(
        reinterpret_cast<const uint16_t*>(m_lastOutput.data()), m_lastOutput.length(),
        buf, 512,
        offsets, limit
    );

    for (size_t i = 0; i < count; ++i) {
        const wchar_t* ptr = reinterpret_cast<const wchar_t*>(buf + offsets[i]);
        results.emplace_back(ptr);
    }
#endif
    return results;
}

bool PhoneticEngine::CommitSuggestion(size_t index) {
    auto suggestions = GetSuggestions(index + 1);
    if (index < suggestions.size()) {
        ApplyDiff(suggestions[index]);
        Reset();
        return true;
    }
    return false;
}

bool PhoneticEngine::ProcessCharacter(char ch, NativeWindowHandle foregroundWnd) {
    if (foregroundWnd != m_lastWindow) {
        Reset();
        m_lastWindow = foregroundWnd;
    }

    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        TryExpandMacro();
        Reset();
        return false;
    }

    m_rawBuffer += ch;

    // Check if macro matches on trigger suffix (e.g. 1500/=)
    if (TryExpandMacro()) {
        return true;
    }

    std::wstring newOutput = m_parser.Convert(m_rawBuffer);
    ApplyDiff(newOutput);
    return true;
}

bool PhoneticEngine::ProcessBackspace(NativeWindowHandle foregroundWnd) {
    if (foregroundWnd != m_lastWindow) {
        Reset();
        m_lastWindow = foregroundWnd;
    }

    if (m_rawBuffer.empty()) {
        return false;
    }

    m_rawBuffer.pop_back();
    std::wstring newOutput = m_rawBuffer.empty() ? L"" : m_parser.Convert(m_rawBuffer);
    ApplyDiff(newOutput);
    return true;
}

bool PhoneticEngine::ProcessKey(
    uint32_t vkCode,
    bool isKeyDown,
    bool isSysKey,
    bool ctrl,
    bool alt,
    bool shift,
    NativeWindowHandle foregroundWnd) {

    (void)isSysKey;

    if (!isKeyDown) {
        return false;
    }

    // Reset buffer if target window changed
    if (foregroundWnd != m_lastWindow) {
        Reset();
        m_lastWindow = foregroundWnd;
    }

    // Allow modifier key combinations (shortcuts) to pass through
    if (ctrl || alt) {
        Reset();
        return false;
    }

#if defined(_WIN32)
    // Space commits the word (expanding macro if active) and passes through
    if (vkCode == VK_SPACE) {
        TryExpandMacro();
        Reset();
        return false;
    }

    // Tab commits the first predictive suggestion candidate if available
    if (vkCode == VK_TAB) {
        if (CommitSuggestion(0)) {
            return true;
        }
        Reset();
        return false;
    }

    // Word boundary / navigation keys commit the current word
    if (vkCode == VK_RETURN || vkCode == VK_TAB || vkCode == VK_ESCAPE ||
        vkCode == VK_LEFT || vkCode == VK_RIGHT || vkCode == VK_UP || vkCode == VK_DOWN ||
        vkCode == VK_HOME || vkCode == VK_END || vkCode == VK_PRIOR || vkCode == VK_NEXT ||
        vkCode == VK_DELETE) {
        Reset();
        return false;
    }

    // Backspace handling
    if (vkCode == VK_BACK) {
        return ProcessBackspace(foregroundWnd);
    }
#else
    if (vkCode == ' ' || vkCode == 0xff08) {
        if (vkCode == 0xff08) {
            return ProcessBackspace(foregroundWnd);
        }
        Reset();
        return false;
    }
#endif

    // Map virtual key to ASCII character
    const char ch = MapVirtualKeyToChar(vkCode, shift);
    if (ch == 0) {
        Reset();
        return false;
    }

    return ProcessCharacter(ch, foregroundWnd);
}

} // namespace bijoy::core

