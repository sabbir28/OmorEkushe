#ifndef UNICODE
#define UNICODE
#endif

#include "MainWindow.h"
#include "LayoutEditor.h"
#include "KeyboardView.h"
#include "KeyEditDialog.h"
#include <commctrl.h>

static HFONT g_hFont = nullptr;

std::vector<editor::LayoutData> MainWindow::s_layouts;
int MainWindow::s_currentLayoutIndex = -1;
HWND MainWindow::s_hCombo = nullptr;
HWND MainWindow::s_hKeyboardView = nullptr;
HWND MainWindow::s_hEditName = nullptr;

constexpr int IDC_KEYBOARD_VIEW = 2001;

bool MainWindow::Register(HINSTANCE hInstance) {
    if (!editor::KeyboardView::Register(hInstance)) {
        return false;
    }

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

    s_hEditName = CreateWindowW(
        L"EDIT", L"",
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

    s_hCombo = CreateWindowW(
        L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        150, 68, 300, 200,
        hwnd,
        reinterpret_cast<HMENU>(IDC_COMBO_LAYOUT),
        hInstance,
        nullptr
    );

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

    s_hKeyboardView = editor::KeyboardView::Create(
        hwnd, hInstance,
        30, 130, 520, 190,
        IDC_KEYBOARD_VIEW
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

void MainWindow::RefreshComboBox(HWND hwnd) {
    SendMessageW(s_hCombo, CB_RESETCONTENT, 0, 0);
    s_layouts = editor::FindLayouts();
    for (const auto& layout : s_layouts) {
        SendMessageW(s_hCombo, CB_ADDSTRING, 0, (LPARAM)layout.name.c_str());
    }
    if (!s_layouts.empty()) {
        SendMessageW(s_hCombo, CB_SETCURSEL, 0, 0);
        s_currentLayoutIndex = 0;
        UpdateLayoutView(hwnd);
    }
}

void MainWindow::UpdateLayoutView(HWND hwnd) {
    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < s_layouts.size()) {
        const auto& layout = s_layouts[s_currentLayoutIndex];
        SetWindowTextW(s_hEditName, layout.name.c_str());
        editor::KeyboardView::SetLayout(s_hKeyboardView, &layout);
    }
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
            MainWindow::RefreshComboBox(hwnd);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == IDC_COMBO_LAYOUT && wmEvent == CBN_SELCHANGE) {
                s_currentLayoutIndex = SendMessageW(s_hCombo, CB_GETCURSEL, 0, 0);
                UpdateLayoutView(hwnd);
                return 0;
            }

            if (wmId == IDC_KEYBOARD_VIEW) {
                int clickedKeyCode = HIWORD(wParam);
                if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < s_layouts.size()) {
                    auto& layout = s_layouts[s_currentLayoutIndex];
                    if (editor::KeyEditDialog::Show(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), clickedKeyCode, &layout)) {
                        editor::KeyboardView::SetLayout(s_hKeyboardView, &layout); // Re-render
                    }
                }
                return 0;
            }

            switch (wmId) {
                case IDC_BTN_NEW:
                    MessageBoxW(hwnd, L"New layout initialized.", L"Keyboard Layout Editor", MB_ICONINFORMATION | MB_OK);
                    return 0;

                case IDC_BTN_SAVE: {
                    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < s_layouts.size()) {
                        wchar_t newName[256];
                        GetWindowTextW(s_hEditName, newName, 256);
                        
                        s_layouts[s_currentLayoutIndex].name = newName;
                        if (s_layouts[s_currentLayoutIndex].saveToFile(s_layouts[s_currentLayoutIndex].path.c_str())) {
                            MessageBoxW(hwnd, L"Layout explicitly saved back to XML!", L"Success", MB_ICONINFORMATION | MB_OK);
                        } else {
                            MessageBoxW(hwnd, L"Failed to save to file!", L"Error", MB_ICONERROR | MB_OK);
                        }
                    }
                    return 0;
                }

                case IDC_BTN_CANCEL:
                    DestroyWindow(hwnd);
                    return 0;
            }
            break;
        }

        case WM_DESTROY:
            MainWindow::OnDestroy(hwnd);
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
