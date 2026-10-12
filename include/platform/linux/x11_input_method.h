#pragma once

#if !defined(_WIN32) && !defined(__APPLE__)
#include "core/platform_types.h"
#include <string>

namespace bijoy::platform::linux {

/// Native X11 Input & Window Management Engine
class X11InputEngine {
public:
    static X11InputEngine& Instance();

    bool Initialize();
    void Shutdown();

    bool IsAvailable() const;

    /// Emits backspaces and Unicode characters to the focused X11 window
    void EmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend);

    /// Returns the currently active window via _NET_ACTIVE_WINDOW
    bijoy::core::NativeWindowHandle GetActiveWindow();

    /// Simulates a raw keycode press/release
    void FakeKeyEvent(unsigned int keycode, bool isDown);

    /// Grabs global hotkeys (e.g. F12 or Ctrl+Alt+B)
    bool GrabGlobalHotkey(int keycode, unsigned int modifiers);
    void UngrabGlobalHotkeys();

private:
    X11InputEngine();
    ~X11InputEngine();

    void* m_display{nullptr};
    bool m_initialized{false};
    bool m_hasXTest{false};
};

} // namespace bijoy::platform::linux
#endif

