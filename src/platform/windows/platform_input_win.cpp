#include "core/platform_input.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

namespace bijoy::core {

    void PlatformEmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend) {
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

    NativeWindowHandle PlatformGetActiveWindow() {
        return ::GetForegroundWindow();
    }

    void PlatformSimulateKey(int keyCode, bool isKeyDown) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(keyCode);
        input.ki.dwFlags = isKeyDown ? 0 : KEYEVENTF_KEYUP;
        input.ki.dwExtraInfo = OMOR_EKUSHE_EXTRA_INFO;
        ::SendInput(1, &input, sizeof(INPUT));
    }

} // namespace bijoy::core
#endif

