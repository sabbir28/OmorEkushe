// =============================================================================
// File   : native_input.cpp
// Module : platform/windows (Synthetic Keyboard Input)
// Purpose: Provides thin wrappers around Win32 SendInput() for injecting
//          keyboard events into the input queue. Used by the keyboard hook
//          to emit Unicode characters (KEYEVENTF_UNICODE) when a layout
//          remapping is active, and for virtual-key based input simulation.
//
//          DoKeyboard()   — sends a scan-code-based input event (Unicode chars)
//          DoKeyboardVk() — sends a virtual-key-based input event (VK codes)
// =============================================================================

#include "platform/windows/native_input.h"

namespace bijoy::platform::windows {

    void DoKeyboard(DWORD flags, int scanCode) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = static_cast<WORD>(scanCode);
        input.ki.dwFlags = flags;
        ::SendInput(1, &input, sizeof(INPUT));
    }

    void DoKeyboardVk(DWORD flags, WORD vk) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vk;
        input.ki.wScan = 0;
        input.ki.dwFlags = flags;
        ::SendInput(1, &input, sizeof(INPUT));
    }

} // namespace bijoy::platform::windows
