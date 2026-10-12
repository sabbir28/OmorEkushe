#if !defined(_WIN32) && !defined(__APPLE__)
#include "core/platform_input.h"
#include "platform/linux/x11_input_method.h"
#include "platform/linux/wayland_input_method.h"

namespace bijoy::core {

void PlatformEmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend) {
    if (platform::linux::X11InputEngine::Instance().IsAvailable()) {
        platform::linux::X11InputEngine::Instance().EmitBackspacesAndChars(backspaces, toSend);
    } else if (platform::linux::WaylandInputMethodEngine::Instance().IsAvailable()) {
        if (backspaces > 0) {
            platform::linux::WaylandInputMethodEngine::Instance().DeleteSurroundingText(-static_cast<int>(backspaces), static_cast<unsigned int>(backspaces));
        }
        if (!toSend.empty()) {
            platform::linux::WaylandInputMethodEngine::Instance().CommitText(toSend);
        }
    }
}

NativeWindowHandle PlatformGetActiveWindow() {
    if (platform::linux::X11InputEngine::Instance().IsAvailable()) {
        return platform::linux::X11InputEngine::Instance().GetActiveWindow();
    }
    return 0;
}

void PlatformSimulateKey(int keyCode, bool isKeyDown) {
    if (platform::linux::X11InputEngine::Instance().IsAvailable()) {
        platform::linux::X11InputEngine::Instance().FakeKeyEvent(static_cast<unsigned int>(keyCode), isKeyDown);
    }
}

} // namespace bijoy::core
#endif

