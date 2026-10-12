#pragma once

#if !defined(_WIN32) && !defined(__APPLE__)
#include "core/platform_types.h"
#include <string>

namespace bijoy::platform::linux {

/// Wayland / IBus Input Method Protocol Engine
class WaylandInputMethodEngine {
public:
    static WaylandInputMethodEngine& Instance();

    bool Initialize();
    void Shutdown();

    bool IsAvailable() const;

    /// Commits text to active Wayland / IBus client
    void CommitText(const std::wstring& utf16Text);

    /// Deletes surrounding characters before cursor
    void DeleteSurroundingText(int offset, unsigned int length);

    /// Sets composition preedit string
    void SetPreeditText(const std::wstring& preedit, int cursorPosition);

    /// Processes client key event
    bool ProcessKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state);

private:
    WaylandInputMethodEngine();
    ~WaylandInputMethodEngine();

    bool m_initialized{false};
    std::wstring m_currentPreedit;
};

} // namespace bijoy::platform::linux
#endif

