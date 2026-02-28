#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include "MainWindow.h"

// Enable modern visual styles natively in MSVC
#if defined(_MSC_VER)
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow) {
    return wWinMain(hInstance, hPrevInstance, GetCommandLineW(), nCmdShow);
}

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PWSTR pCmdLine,
    int nCmdShow
) {
    // Suppress unused parameter warnings
    (void)hPrevInstance;
    (void)pCmdLine;

    // Initialize common controls for modern visual styles
    INITCOMMONCONTROLSEX icc = {0};
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    if (!MainWindow::Register(hInstance)) {
        MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    HWND hwnd = MainWindow::Create(hInstance, nCmdShow);
    if (!hwnd) {
        MessageBoxW(nullptr, L"Failed to create main window.", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    MSG msg = {0};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
