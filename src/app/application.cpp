// =============================================================================
// File   : application.cpp
// Module : app (Application Lifecycle Manager)
// Purpose: Implements the state-machine-based Application class that controls
//          the full lifecycle of Omor Ekushe: initialization, message loop
//          execution, and graceful shutdown.
//
//          Lifecycle states:
//            Uninitialized -> Initializing -> Running -> ShuttingDown -> Terminated
//
//          Initialization sequence:
//            1. Install crash diagnostics (VEH, SEH, MiniDump handlers)
//            2. Initialize Win32 common controls (comctl32)
//            3. Discover keyboard layout XML files from data/ directories
//            4. Install global low-level keyboard hook (WH_KEYBOARD_LL)
//            5. Create the main toolbar window (native Win32)
//            6. Show animated splash screen with background init tasks
//
//          The main message loop runs in Run() and is shielded by try-catch
//          barriers that invoke crash reporting on unhandled C++ exceptions.
// =============================================================================

#include "app/application.h"
#include "error/crash_handler.h"
#include "error/raii_handle.h"
#include "core/app_state.h"
#include "core/keyboard_hook_service.h"
#include "core/layout_discovery.h"
#include "platform/windows/main_window.h"
#include "platform/windows/registration_dialog.h"
#include "platform/windows/splash_screen.h"
#include "utils/system_utils.h"

#include <commctrl.h>
#include <exception>

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

    // 1. Install ultra crash diagnostic handlers
    error::InstallCrashDiagnostics();

    // 2. Initialize Common Controls
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    if (!InitCommonControlsEx(&icc)) {
        // Fallback or warning, not fatal for standard Win32 windows
    }

    // 3. Discover application installation directory
    const std::wstring appDir = core::GetAppDirectory();

    // 4. Load and validate keyboard layout definitions
    if (!core::FindLayouts(core::g_layouts, appDir)) {
        MessageBoxW(
            nullptr,
            L"No layout XML files found. Expected paths include data\\layout.",
            L"Omor Ekushe Error",
            MB_OK | MB_ICONERROR);
        m_state = ApplicationState::Terminated;
        return error::Error(2, "Failed to load layout XML definitions");
    }

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

    // 8. Registration gate (Bypassed per request)
    // const int registrationResult = platform::windows::ShowRegistrationDialog(m_mainWindow);
    // if (registrationResult == 0) {
    //     Shutdown();
    //     m_state = ApplicationState::Terminated;
    //     return error::Error(0, "Registration cancelled");
    // }

    // 9. Startup options & splash screen
    m_startupOptions = std::make_shared<core::StartupOptions>();

    HWND splashWindow = platform::windows::ShowSplashScreen(
        m_hInstance,
        m_mainWindow,
        [this]() {
            *m_startupOptions = core::LoadStartupOptions();

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

    if (!splashWindow) {
        ShowWindow(m_mainWindow, SW_SHOW);
    }

    m_state = ApplicationState::Running;
    return error::Status();
}

int Application::Run() {
    if (m_state != ApplicationState::Running) {
        return -1;
    }

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
}

void Application::Shutdown() {
    if (m_state == ApplicationState::ShuttingDown || m_state == ApplicationState::Terminated) {
        return;
    }

    m_state = ApplicationState::ShuttingDown;

    // Teardown main window
    if (m_mainWindow && IsWindow(m_mainWindow)) {
        DestroyWindow(m_mainWindow);
        m_mainWindow = nullptr;
    }

    // Teardown keyboard hook
    if (m_hookInstalled) {
        core::UninstallKeyboardHook();
        m_hookInstalled = false;
    }

    m_state = ApplicationState::Terminated;
}

} // namespace bijoy::app
