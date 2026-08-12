// =============================================================================
// File   : main.cpp
// Module : app (Application Entry Point)
// Purpose: Top-level process entry point for Omor Ekushe.
//          Contains WinMain/wWinMain definitions and the outermost SEH __try
//          barrier. All C++ object construction and destruction (RAII) is
//          isolated in RunApplicationHelper() because SEH and C++ object
//          unwinding cannot coexist in the same function scope on MSVC.
//
//          Execution flow:
//            WinMain -> wWinMain -> __try { RunApplicationHelper() }
//                                         -> Application::Initialize()
//                                         -> Application::Run()
// =============================================================================

#include "app/application.h"
#include "error/crash_handler.h"

#include <windows.h>

// -----------------------------------------------------------------------------
// Helper function to encapsulate C++ object unwinding safely outside SEH __try
// -----------------------------------------------------------------------------
static int RunApplicationHelper(HINSTANCE hInstance, int nCmdShow) {
    bijoy::app::Application app;
    const bijoy::error::Status initStatus = app.Initialize(hInstance, nCmdShow);
    if (!initStatus.IsOk()) {
        // Early return or cancellation (e.g. registration dialog closed)
        return 0;
    }

    return app.Run();
}

// -----------------------------------------------------------------------------
// Forward declarations for WinMain entry points
// -----------------------------------------------------------------------------
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR,
    int nCmdShow) {
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
}

// -----------------------------------------------------------------------------
// Primary application entry point (Unicode)
// Ultra-level exception barrier surrounding Application Lifecycle Manager
// -----------------------------------------------------------------------------
int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    // Guaranteed top-level process exception barrier
    __try {
        return RunApplicationHelper(hInstance, nCmdShow);
    }
    __except (bijoy::error::GenerateCrashReport(GetExceptionInformation(), L"Top-Level Process SEH Violation"), EXCEPTION_EXECUTE_HANDLER) {
        return 1;
    }
}