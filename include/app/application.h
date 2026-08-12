#pragma once

#include <memory>
#if defined(_WIN32)
#include <windows.h>
#endif

#include "error/raii_handle.h"
#include "error/result.h"
#include "core/startup_options.h"

namespace bijoy::app {

enum class ApplicationState {
    Uninitialized,
    Initializing,
    Running,
    ShuttingDown,
    Terminated
};

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // -------------------------------------------------------------------------
    // Initializes runtime, crash handler, layout engines, and window subsystems
    // -------------------------------------------------------------------------
    [[nodiscard]] error::Status Initialize(HINSTANCE hInstance, int nCmdShow);

    // -------------------------------------------------------------------------
    // Enters guarded Win32 message loop
    // -------------------------------------------------------------------------
    [[nodiscard]] int Run();

    // -------------------------------------------------------------------------
    // Performs clean shutdown & resource release in exact reverse order
    // -------------------------------------------------------------------------
    void Shutdown();

    [[nodiscard]] ApplicationState GetState() const noexcept { return m_state; }
    [[nodiscard]] HWND GetMainWindow() const noexcept { return m_mainWindow; }

private:
    HINSTANCE m_hInstance{nullptr};
    HWND m_mainWindow{nullptr};
    ApplicationState m_state{ApplicationState::Uninitialized};
    std::shared_ptr<bijoy::core::StartupOptions> m_startupOptions;
    bool m_hookInstalled{false};
};

} // namespace bijoy::app
