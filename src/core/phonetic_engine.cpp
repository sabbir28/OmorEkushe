#include "core/phonetic_engine.h"
#include "core/platform_input.h"

#include <vector>

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
    while (common < oldOutput.length() &&
           common < newOutput.length() &&
           oldOutput[common] == newOutput[common]) {
        ++common;
    }

    result.commonPrefixLength = common;
    result.backspaceCount = oldOutput.length() - common;
    result.toSend = newOutput.substr(common);
    return result;
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

bool PhoneticEngine::ProcessCharacter(char ch, NativeWindowHandle foregroundWnd) {
    if (foregroundWnd != m_lastWindow) {
        Reset();
        m_lastWindow = foregroundWnd;
    }

    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        Reset();
        return false;
    }

    m_rawBuffer += ch;
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
    // Space commits the word and passes through
    if (vkCode == VK_SPACE) {
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

