#include "core/app_state.h"
#include "core/keyboard_hook_service.h"
#include "core/layout_discovery.h"
#include "core/startup_options.h"
#include "platform/windows/main_window.h"
#include "platform/windows/registration_dialog.h"
#include "platform/windows/splash_screen.h"

#include <commctrl.h>
#include <memory>
#include <windows.h>

// -----------------------------------------------------------------------------
// Unicode entry point forward declaration
// Ensures consistent startup path regardless of subsystem configuration
// -----------------------------------------------------------------------------
int WINAPI wWinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR lpCmdLine,
        int nCmdShow);

// -----------------------------------------------------------------------------
// ANSI entry point
// Delegates immediately to Unicode version to avoid duplicated logic
// -----------------------------------------------------------------------------
int WINAPI WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR,
        int nCmdShow) {
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
}

// -----------------------------------------------------------------------------
// Primary application entry point (Unicode)
// Responsible for:
// - Runtime initialization
// - UI bootstrap
// - Keyboard hook lifecycle
// - Message pump ownership
// -----------------------------------------------------------------------------
int WINAPI wWinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPWSTR lpCmdLine,
        int nCmdShow) {

    // Explicitly mark unused parameters to avoid warnings
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // ---------------------------------------------------------------------------
    // Initialize common Windows controls (buttons, dialogs, etc.)
    // Required before creating any UI that relies on comctl32
    // ---------------------------------------------------------------------------
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    // ---------------------------------------------------------------------------
    // Discover application installation directory
    // Used as the root for layout and resource discovery
    // ---------------------------------------------------------------------------
    const std::wstring appDir = bijoy::core::GetAppDirectory();

    // Load and validate keyboard layout definitions
    if (!bijoy::core::FindLayouts(bijoy::core::g_layouts, appDir)) {
        MessageBoxW(
                nullptr,
                L"No layout XML files found. Expected paths include data\\layout.",
                L"Omor Ekushe",
                MB_OK | MB_ICONERROR);
        return 1;
    }

    // ---------------------------------------------------------------------------
    // Install low-level keyboard hook
    // Critical for intercepting and remapping keystrokes
    // ---------------------------------------------------------------------------
    if (!bijoy::core::InstallKeyboardHook(hInstance)) {
        MessageBoxW(
                nullptr,
                L"Failed to install keyboard hook.",
                L"Omor Ekushe",
                MB_OK | MB_ICONERROR);
        return 1;
    }

    // Signal that layout data is fully initialized and safe to consume
    bijoy::core::SetLayoutsReady(true);

    // ---------------------------------------------------------------------------
    // Create main application window
    // Window is initially hidden pending registration and startup logic
    // ---------------------------------------------------------------------------
    HWND mainWindow = bijoy::platform::windows::CreateMainWindow(hInstance);
    if (!mainWindow) {
        return 1;
    }
    ShowWindow(mainWindow, SW_HIDE);

    // ---------------------------------------------------------------------------
    // Registration / licensing gate
    // Application does not proceed unless registration succeeds
    // ---------------------------------------------------------------------------
    HWND splashWindow = nullptr;
    const int registrationResult =
            bijoy::platform::windows::ShowRegistrationDialog(mainWindow);

    // Registration cancelled or failed — clean shutdown
    if (registrationResult == 0) {
        DestroyWindow(mainWindow);
        bijoy::core::UninstallKeyboardHook();
        return 0;
    }

    // ---------------------------------------------------------------------------
    // Startup options are shared across splash callbacks
    // Loaded asynchronously during splash screen display
    // ---------------------------------------------------------------------------
    auto startupOptions =
            std::make_shared<bijoy::core::StartupOptions>();

    splashWindow = bijoy::platform::windows::ShowSplashScreen(
            hInstance,
            mainWindow,

            // Phase 1: Load persisted startup configuration
            [startupOptions]() {
                *startupOptions = bijoy::core::LoadStartupOptions();

                // Restore window position, keeping the bar pinned to the top edge.
                bijoy::platform::windows::SetMainWindowInitialPosition(
                        startupOptions->mainWindowLeft,
                        startupOptions->mainWindowTop);

                // Apply default keyboard layout if valid
                if (startupOptions->defaultLayout >= 0 &&
                    startupOptions->defaultLayout < bijoy::core::GetLayoutCount()) {
                    bijoy::core::SetCurrentLayout(
                            startupOptions->defaultLayout);
                }
            },

            // Phase 2: Decide window visibility based on startup mode
            [mainWindow, startupOptions]() {
                const bool shouldStartHidden =
                        startupOptions->applicationMode == 3 ||
                        (startupOptions->applicationMode != 2 &&
                         startupOptions->trayMode);

                ShowWindow(
                        mainWindow,
                        shouldStartHidden ? SW_HIDE : SW_SHOW);
            });

    // Fallback: if splash screen creation failed, show main window immediately
    if (!splashWindow) {
        ShowWindow(mainWindow, SW_SHOW);
    }

    // ---------------------------------------------------------------------------
    // Main message loop
    // Owns the UI thread until application termination
    // ---------------------------------------------------------------------------
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ---------------------------------------------------------------------------
    // Graceful shutdown
    // Ensure keyboard hook is always removed
    // ---------------------------------------------------------------------------
    bijoy::core::UninstallKeyboardHook();
    return static_cast<int>(msg.wParam);
}