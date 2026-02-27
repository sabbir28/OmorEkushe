#ifndef UNICODE
#define UNICODE
#endif

#include "LayoutEditor.h"
#include <commctrl.h>

HINSTANCE g_hInstance = nullptr;

int WINAPI wWinMain(
        HINSTANCE hInstance,
        HINSTANCE,
        PWSTR,
        int nCmdShow
) {
    g_hInstance = hInstance;

    INITCOMMONCONTROLSEX icc{
            sizeof(icc),
            ICC_WIN95_CLASSES
    };
    InitCommonControlsEx(&icc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LayoutEditorWindow";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
            0,
            wc.lpszClassName,
            L"Keyboard Layout Editor",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            APP_WIDTH, APP_HEIGHT,
            nullptr, nullptr,
            hInstance,
            nullptr
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void CreateMainControls(HWND hwnd) {
    CreateWindowW(
            L"STATIC", L"Layout Name:",
            WS_CHILD | WS_VISIBLE,
            20, 20, 120, 20,
            hwnd, nullptr, g_hInstance, nullptr
    );

    CreateWindowW(
            L"EDIT", L"My New Layout",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            150, 18, 300, 24,
            hwnd,
            reinterpret_cast<HMENU>(IDC_EDIT_NAME),
            g_hInstance,
            nullptr
    );

    CreateWindowW(
            L"STATIC", L"Base Layout:",
            WS_CHILD | WS_VISIBLE,
            20, 70, 120, 20,
            hwnd, nullptr, g_hInstance, nullptr
    );

    HWND combo = CreateWindowW(
            L"COMBOBOX", nullptr,
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            150, 68, 300, 200,
            hwnd,
            reinterpret_cast<HMENU>(IDC_COMBO_LAYOUT),
            g_hInstance,
            nullptr
    );

    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Bijoy Classic");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Bijoy Unicode");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Probhat");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Avro");
    SendMessageW(combo, CB_SETCURSEL, 0, 0);

    CreateWindowW(
            L"BUTTON", L"New Layout",
            WS_CHILD | WS_VISIBLE,
            40, 140, 140, 36,
            hwnd,
            reinterpret_cast<HMENU>(IDC_BTN_NEW),
            g_hInstance,
            nullptr
    );

    CreateWindowW(
            L"BUTTON", L"Save",
            WS_CHILD | WS_VISIBLE,
            200, 140, 140, 36,
            hwnd,
            reinterpret_cast<HMENU>(IDC_BTN_SAVE),
            g_hInstance,
            nullptr
    );

    CreateWindowW(
            L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE,
            360, 140, 140, 36,
            hwnd,
            reinterpret_cast<HMENU>(IDC_BTN_CANCEL),
            g_hInstance,
            nullptr
    );
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM) {
switch (msg) {
    case WM_CREATE:
        CreateMainControls(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_NEW:
            MessageBoxW(hwnd, L"New layout initialized.", L"Info", MB_OK);
            return 0;

        case IDC_BTN_SAVE:
            MessageBoxW(hwnd, L"Persist layer not wired yet.", L"Info", MB_OK);
            return 0;

        case IDC_BTN_CANCEL:
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
}
return DefWindowProcW(hwnd, msg, wParam, 0);
}
