#pragma once

#include "core/phonetic_parser.h"
#include "core/platform_types.h"
#include <string>

namespace bijoy::core {

class PhoneticEngine {
public:
    PhoneticEngine();
    explicit PhoneticEngine(const PhoneticParser& parser);

    /// Processes a physical key press.
    /// Returns true if the key event was consumed and transliterated.
    /// Returns false if the key should pass through to the system.
    bool ProcessKey(
        uint32_t vkCode,
        bool isKeyDown,
        bool isSysKey,
        bool ctrl,
        bool alt,
        bool shift,
        NativeWindowHandle foregroundWnd = kInvalidWindowHandle);

    /// Directly processes an incoming ASCII character (used by Input Method Kit / IBus).
    bool ProcessCharacter(char ch, NativeWindowHandle foregroundWnd = kInvalidWindowHandle);

    /// Directly processes a backspace event.
    bool ProcessBackspace(NativeWindowHandle foregroundWnd = kInvalidWindowHandle);

    /// Resets the current word buffer (commits on-screen text).
    void Reset();

    /// Converts a physical key to its ASCII character representation.
    char MapVirtualKeyToChar(uint32_t vkCode, bool shift) const;

    const std::string& GetRawBuffer() const { return m_rawBuffer; }
    const std::wstring& GetLastOutput() const { return m_lastOutput; }
    const PhoneticParser& GetParser() const { return m_parser; }

    /// Helper for testing diff calculation without sending input
    struct DiffResult {
        size_t commonPrefixLength = 0;
        size_t backspaceCount = 0;
        std::wstring toSend;
    };
    static DiffResult ComputeDiff(const std::wstring& oldOutput, const std::wstring& newOutput);

private:
    void ApplyDiff(const std::wstring& newOutput);
    void EmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend);

    PhoneticParser m_parser;
    std::string m_rawBuffer;
    std::wstring m_lastOutput;
    NativeWindowHandle m_lastWindow{kInvalidWindowHandle};
};

extern PhoneticEngine g_phoneticEngine;

} // namespace bijoy::core

