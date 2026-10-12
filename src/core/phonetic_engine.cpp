#include "core/phonetic_engine.h"
#include "platform/windows/native_input.h"

#include <vector>

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
    if (backspaces == 0 && toSend.empty()) {
        return;
    }

    std::vector<INPUT> inputs;
    inputs.reserve(backspaces * 2 + toSend.length() * 2);

    for (size_t i = 0; i < backspaces; ++i) {
        INPUT down = {};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = VK_BACK;
        down.ki.wScan = 0;
        down.ki.dwFlags = 0;
        down.ki.dwExtraInfo = OMOR_EKUSHE_EXTRA_INFO;

        INPUT up = down;
        up.ki.dwFlags = KEYEVENTF_KEYUP;

        inputs.push_back(down);
        inputs.push_back(up);
    }

    for (wchar_t c : toSend) {
        INPUT down = {};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = 0;
        down.ki.wScan = static_cast<WORD>(c);
        down.ki.dwFlags = KEYEVENTF_UNICODE;
        down.ki.dwExtraInfo = OMOR_EKUSHE_EXTRA_INFO;

        INPUT up = down;
        up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

        inputs.push_back(down);
        inputs.push_back(up);
    }

    if (!inputs.empty()) {
        ::SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    }
}

void PhoneticEngine::ApplyDiff(const std::wstring& newOutput) {
    DiffResult diff = ComputeDiff(m_lastOutput, newOutput);
    EmitBackspacesAndChars(diff.backspaceCount, diff.toSend);
    m_lastOutput = newOutput;
}

char PhoneticEngine::MapVirtualKeyToChar(DWORD vkCode, bool shift) const {
    if (vkCode >= 'A' && vkCode <= 'Z') {
        const bool caps = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
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

    return 0;
}

bool PhoneticEngine::ProcessKey(
    DWORD vkCode,
    bool isKeyDown,
    bool isSysKey,
    bool ctrl,
    bool alt,
    bool shift,
    HWND foregroundWnd) {

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
        if (m_rawBuffer.empty()) {
            return false;
        }

        m_rawBuffer.pop_back();
        std::wstring newOutput = m_rawBuffer.empty() ? L"" : m_parser.Convert(m_rawBuffer);
        ApplyDiff(newOutput);
        return true;
    }

    // Map virtual key to ASCII character
    const char ch = MapVirtualKeyToChar(vkCode, shift);
    if (ch == 0) {
        // Unknown or non-printable key
        Reset();
        return false;
    }

    // Append character to raw buffer and transliterate
    m_rawBuffer += ch;
    std::wstring newOutput = m_parser.Convert(m_rawBuffer);
    ApplyDiff(newOutput);
    return true;
}

} // namespace bijoy::core

