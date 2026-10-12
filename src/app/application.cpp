// =============================================================================
// File   : application.cpp
// Module : app (Application Lifecycle Manager)
// Purpose: Implements the state-machine-based Application class that controls
//          the full lifecycle of Omor Ekushe across Windows, Linux, and macOS.
// =============================================================================

#include "app/application.h"
#include "error/crash_handler.h"
#include "error/raii_handle.h"
#include "core/app_state.h"
#include "core/keyboard_hook_service.h"
#include "core/layout_discovery.h"
#include "utils/system_utils.h"

#include <exception>

#if defined(_WIN32)
#include "platform/windows/main_window.h"
#include "platform/windows/registration_dialog.h"
#include "platform/windows/splash_screen.h"
#include <commctrl.h>
#elif defined(__APPLE__)
namespace bijoy::platform::macos { void CreateStatusBarItem(); }
#else
#include "platform/linux/main_window.h"
#include "platform/linux/x11_input_method.h"
#include "platform/linux/wayland_input_method.h"
#endif

namespace bijoy::app {

Application::Application()
    : m_state(ApplicationState::Uninitialized) {
}

Application::~Application() {
    Shutdown();
}

error::Status Application::Initialize(HINSTANCE hInstance, int nCmdShow) {
    (void)nCmdShow;

    if (m_state != ApplicationState::Uninitialized) {
        return error::Error(1, "Application already initialized");
    }

    m_state = ApplicationState::Initializing;
    m_hInstance = hInstance;

    // 1. Install crash diagnostic handlers
    error::InstallCrashDiagnostics();

#if defined(_WIN32)
    // 2. Initialize Common Controls on Windows
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_HOTKEY_CLASS};
    InitCommonControlsEx(&icc);
#endif

    // 3. Discover application installation directory
    const std::wstring appDir = core::GetAppDirectory();

    // 4. Load keyboard layout definitions
    core::FindLayouts(core::g_layouts, appDir);

#if defined(_WIN32)
    // 5. Install low-level keyboard hook
    if (!core::InstallKeyboardHook(m_hInstance)) {
        MessageBoxW(
            nullptr,
            L"Failed to install low-level keyboard hook.",
            L"Omor Ekushe Error",
            MB_OK | MB_ICONERROR);
        m_state = ApplicationState::Terminated;
        return error::Error(3, "Failed to install keyboard hook");
    }
    m_hookInstalled = true;

    // 6. Signal layout readiness
    core::SetLayoutsReady(true);

    // 7. Create main application window
    m_mainWindow = platform::windows::CreateMainWindow(m_hInstance);
    if (!m_mainWindow) {
        Shutdown();
        return error::Error(4, "Failed to create main application window");
    }
    ShowWindow(m_mainWindow, SW_HIDE);

    // 8. Startup options & splash screen
    m_startupOptions = std::make_shared<core::StartupOptions>();
    *m_startupOptions = core::LoadStartupOptions();
    core::SetPhoneticToggleShortcut(
        m_startupOptions->phoneticToggleKey,
        m_startupOptions->phoneticToggleCtrl,
        m_startupOptions->phoneticToggleAlt,
        m_startupOptions->phoneticToggleShift);

    HWND splashWindow = nullptr;
    if (m_startupOptions->showSplash) {
        splashWindow = platform::windows::ShowSplashScreen(
            m_hInstance,
            m_mainWindow,
            [this]() {
                platform::windows::SetMainWindowInitialPosition(
                    m_startupOptions->mainWindowLeft,
                    m_startupOptions->mainWindowTop);

                if (m_startupOptions->defaultLayout >= 0 &&
                    m_startupOptions->defaultLayout < core::GetLayoutCount()) {
                    core::SetCurrentLayout(m_startupOptions->defaultLayout);
                }
            },
            [this]() {
                const bool shouldStartHidden =
                    m_startupOptions->applicationMode == 3 ||
                    (m_startupOptions->applicationMode != 2 && m_startupOptions->trayMode);

                ShowWindow(m_mainWindow, shouldStartHidden ? SW_HIDE : SW_SHOW);
            });
    } else {
        platform::windows::SetMainWindowInitialPosition(
            m_startupOptions->mainWindowLeft,
            m_startupOptions->mainWindowTop);

        if (m_startupOptions->defaultLayout >= 0 &&
            m_startupOptions->defaultLayout < core::GetLayoutCount()) {
            core::SetCurrentLayout(m_startupOptions->defaultLayout);
        }

        const bool shouldStartHidden =
            m_startupOptions->applicationMode == 3 ||
            (m_startupOptions->applicationMode != 2 && m_startupOptions->trayMode);

        ShowWindow(m_mainWindow, shouldStartHidden ? SW_HIDE : SW_SHOW);
    }

    if (!splashWindow && m_startupOptions->showSplash) {
        ShowWindow(m_mainWindow, SW_SHOW);
    }
#elif defined(__APPLE__)
    bijoy::platform::macos::CreateStatusBarItem();
    core::SetLayoutsReady(true);
#else
    bijoy::platform::linux::X11InputEngine::Instance().Initialize();
    bijoy::platform::linux::WaylandInputMethodEngine::Instance().Initialize();
    bijoy::platform::linux::CreateMainWindow();
    core::SetLayoutsReady(true);
#endif

    m_state = ApplicationState::Running;
    return error::Status();
}

int Application::Run() {
    if (m_state != ApplicationState::Running) {
        return -1;
    }

#if defined(_WIN32)
    MSG msg = {};
    try {
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } catch (const std::exception& e) {
        wchar_t wbuf[512] = {};
        swprintf_s(wbuf, L"Unhandled Exception in Main Message Loop: %hs", e.what());
        error::GenerateCrashReport(nullptr, wbuf);
    } catch (...) {
        error::GenerateCrashReport(nullptr, L"Unhandled C++ Exception in Main Message Loop");
    }

    Shutdown();
    return static_cast<int>(msg.wParam);
#else
    return 0;
#endif
}

void Application::Shutdown() {
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated) {
        return;
    }

    m_state = ApplicationState::ShuttingDown;

#if defined(_WIN32)
    if (m_mainWindow && IsWindow(m_mainWindow)) {
        DestroyWindow(m_mainWindow);
        m_mainWindow = nullptr;
    }

    if (m_hookInstalled) {
        core::UninstallKeyboardHook();
        m_hookInstalled = false;
    }
#elif defined(__APPLE__)
    // macOS teardown
#else
    bijoy::platform::linux::X11InputEngine::Instance().Shutdown();
    bijoy::platform::linux::WaylandInputMethodEngine::Instance().Shutdown();
#endif

    m_state = ApplicationState::Terminated;
}

} // namespace bijoy::app

