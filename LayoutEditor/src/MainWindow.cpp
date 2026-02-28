#ifndef UNICODE
#define UNICODE
#endif

#include "MainWindow.h"
#include "LayoutEditor.h"
#include <commctrl.h>

static HFONT g_hFont = nullptr;

bool MainWindow::Register(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LayoutEditorWindow";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    
    return RegisterClassExW(&wc) != 0;
}

HWND MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    HWND hwnd = CreateWindowExW(
        0,
        L"LayoutEditorWindow",
        L"Keyboard Layout Editor",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        APP_WIDTH, APP_HEIGHT,
        nullptr, nullptr,
        hInstance,
        nullptr
    );

    if (hwnd) {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);
    }
    return hwnd;
}

void MainWindow::ApplySystemFont(HWND hwnd) {
    if (!g_hFont) {
        NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
        if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
            g_hFont = CreateFontIndirectW(&ncm.lfMessageFont);
        } else {
            g_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        }
    }
    
    // Apply to all children
    EnumChildWindows(hwnd, [](HWND child, LPARAM lParam) -> BOOL {
        SendMessageW(child, WM_SETFONT, (WPARAM)lParam, TRUE);
        return TRUE;
    }, (LPARAM)g_hFont);
}

void MainWindow::CreateControls(HWND hwnd, HINSTANCE hInstance) {
    CreateWindowW(
        L"STATIC", L"Layout Name:",
        WS_CHILD | WS_VISIBLE,
        20, 20, 120, 20,
        hwnd, nullptr, hInstance, nullptr
    );

    CreateWindowW(
        L"EDIT", L"My New Layout",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        150, 18, 300, 24,
        hwnd,
        reinterpret_cast<HMENU>(IDC_EDIT_NAME),
        hInstance,
        nullptr
    );

    CreateWindowW(
        L"STATIC", L"Base Layout:",
        WS_CHILD | WS_VISIBLE,
        20, 70, 120, 20,
        hwnd, nullptr, hInstance, nullptr
    );

    HWND combo = CreateWindowW(
        L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        150, 68, 300, 200,
        hwnd,
        reinterpret_cast<HMENU>(IDC_COMBO_LAYOUT),
        hInstance,
        nullptr
    );

    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Bijoy Classic");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Bijoy Unicode");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Probhat");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Avro");
    SendMessageW(combo, CB_SETCURSEL, 0, 0);

    // Group Box for Keyboard View Placeholder
    CreateWindowW(
        L"BUTTON", L"Keyboard Layout Preview",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 110, 540, 220,
        hwnd,
        nullptr,
        hInstance,
        nullptr
    );

    CreateWindowW(
        L"STATIC", L"Visual preview will be rendered here...",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        150, 200, 280, 20,
        hwnd, nullptr, hInstance, nullptr
    );

    int btnY = APP_HEIGHT - 80;

    CreateWindowW(
        L"BUTTON", L"New Layout",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        40, btnY, 120, 30,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_NEW),
        hInstance,
        nullptr
    );

    CreateWindowW(
        L"BUTTON", L"Save",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        180, btnY, 120, 30,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_SAVE),
        hInstance,
        nullptr
    );

    CreateWindowW(
        L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        320, btnY, 120, 30,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_CANCEL),
        hInstance,
        nullptr
    );
    
    ApplySystemFont(hwnd);
}

void MainWindow::OnDestroy(HWND hwnd) {
    if (g_hFont) {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
    PostQuitMessage(0);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            LPCREATESTRUCTW cs = reinterpret_cast<LPCREATESTRUCTW>(lParam);
            MainWindow::CreateControls(hwnd, cs->hInstance);
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BTN_NEW:
                    MessageBoxW(hwnd, L"New layout initialized.", L"Keyboard Layout Editor", MB_ICONINFORMATION | MB_OK);
                    return 0;

                case IDC_BTN_SAVE:
                    MessageBoxW(hwnd, L"Persist layer not implemented yet.", L"Keyboard Layout Editor", MB_ICONINFORMATION | MB_OK);
                    return 0;

                case IDC_BTN_CANCEL:
                    DestroyWindow(hwnd);
                    return 0;
            }
            break;

        case WM_DESTROY:
            MainWindow::OnDestroy(hwnd);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
