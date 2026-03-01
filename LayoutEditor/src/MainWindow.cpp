#ifndef UNICODE
#define UNICODE
#endif

#include "MainWindow.h"
#include "LayoutEditor.h"
#include "KeyboardView.h"
#include "KeyEditDialog.h"
#include <commctrl.h>
#include <shlwapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

static HFONT g_hFont = nullptr;

std::vector<editor::LayoutData> MainWindow::s_layouts;
int MainWindow::s_currentLayoutIndex = -1;
HWND MainWindow::s_hCombo = nullptr;
HWND MainWindow::s_hKeyboardView = nullptr;
HWND MainWindow::s_hEditName = nullptr;

constexpr int IDC_KEYBOARD_VIEW = 2001;
constexpr int IDC_BTN_ICON = 2002;
constexpr int IDC_STATIC_LABEL1 = 2003;
constexpr int IDC_STATIC_LABEL2 = 2004;
constexpr int IDC_GROUP_KEYBOARD = 2005;

static HWND s_hLabel1 = nullptr;
static HWND s_hLabel2 = nullptr;
static HWND s_hBtnIcon = nullptr;
static HWND s_hBtnNew = nullptr;
static HWND s_hBtnSave = nullptr;
static HWND s_hBtnCancel = nullptr;
static HWND s_hGroupBox = nullptr;

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
    s_hLabel1 = CreateWindowW(
        L"STATIC", L"Layout Name:",
        WS_CHILD | WS_VISIBLE,
        20, 20, 120, 20,
        hwnd, reinterpret_cast<HMENU>(IDC_STATIC_LABEL1), hInstance, nullptr
    );

    s_hEditName = CreateWindowW(
        L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        150, 18, 220, 24,
        hwnd,
        reinterpret_cast<HMENU>(IDC_EDIT_NAME),
        hInstance,
        nullptr
    );
    
    s_hBtnIcon = CreateWindowW(
        L"BUTTON", L"Icon...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        380, 17, 70, 26,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_ICON),
        hInstance,
        nullptr
    );

    s_hLabel2 = CreateWindowW(
        L"STATIC", L"Base Layout:",
        WS_CHILD | WS_VISIBLE,
        20, 70, 120, 20,
        hwnd, reinterpret_cast<HMENU>(IDC_STATIC_LABEL2), hInstance, nullptr
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

    // Group Box for Keyboard View
    s_hGroupBox = CreateWindowW(
        L"BUTTON", L"Keyboard Layout Preview",
        WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        20, 110, 540, 220,
        hwnd,
        reinterpret_cast<HMENU>(IDC_GROUP_KEYBOARD),
        hInstance,
        nullptr
    );

    s_hKeyboardView = editor::KeyboardView::Create(
        hwnd, hInstance,
        30, 130, 520, 190,
        IDC_KEYBOARD_VIEW
    );

    s_hBtnNew = CreateWindowW(
        L"BUTTON", L"New Layout",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        40, 370, 120, 30,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_NEW),
        hInstance,
        nullptr
    );

    s_hBtnSave = CreateWindowW(
        L"BUTTON", L"Save",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        180, 370, 120, 30,
        hwnd,
        reinterpret_cast<HMENU>(IDC_BTN_SAVE),
        hInstance,
        nullptr
    );

    s_hBtnCancel = CreateWindowW(
        L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        320, 370, 120, 30,
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

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            // Padding and margins
            int margin = 20;
            int labelWidth = 120;
            int controlH = 24;
            int btnNetW = 120;
            int btnH = 32;

            // Layout row 1: [Label] [Editbox] [IconBtn]
            int iconBtnW = 80;
            int editW = width - labelWidth - iconBtnW - (margin * 4);
            if (editW < 100) editW = 100;
            
            MoveWindow(s_hLabel1, margin, margin + 2, labelWidth, controlH, TRUE);
            MoveWindow(s_hEditName, margin + labelWidth + 10, margin, editW, controlH + 4, TRUE);
            MoveWindow(s_hBtnIcon, width - margin - iconBtnW, margin - 1, iconBtnW, controlH + 6, TRUE);

            // Layout row 2: [Label] [Combo]
            int comboW = width - labelWidth - (margin * 3);
            MoveWindow(s_hLabel2, margin, margin * 2 + controlH + 10, labelWidth, controlH, TRUE);
            MoveWindow(s_hCombo, margin + labelWidth + 10, margin * 2 + controlH + 8, comboW, 200, TRUE);

            // Bottom Buttons: [New] [Save] [Cancel]
            int btnY = height - margin - btnH;
            int btnSpacing = 10;
            MoveWindow(s_hBtnNew, margin, btnY, btnNetW, btnH, TRUE);
            MoveWindow(s_hBtnSave, margin + btnNetW + btnSpacing, btnY, btnNetW, btnH, TRUE);
            MoveWindow(s_hBtnCancel, width - margin - btnNetW, btnY, btnNetW, btnH, TRUE);

            // Keyboard View Area
            int kbY = margin * 2 + controlH * 2 + 40;
            int kbH = btnY - kbY - margin - 10;
            if (kbH < 100) kbH = 100;

            MoveWindow(s_hGroupBox, margin, kbY, width - (margin * 2), kbH, TRUE);
            // Keyboard view inside group box with small padding
            MoveWindow(s_hKeyboardView, margin + 10, kbY + 20, width - (margin * 2) - 20, kbH - 30, TRUE);

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
                case IDC_BTN_ICON: {
                    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < s_layouts.size()) {
                        wchar_t filename[MAX_PATH] = {};
                        OPENFILENAMEW ofn = {};
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hwnd;
                        ofn.lpstrFile = filename;
                        ofn.nMaxFile = MAX_PATH;
                        ofn.lpstrFilter = L"Icons (*.ico)\0*.ico\0All Files (*.*)\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                        
                        if (GetOpenFileNameW(&ofn)) {
                            // Only store filename, not full path
                            s_layouts[s_currentLayoutIndex].iconName = PathFindFileNameW(filename);
                            MessageBoxW(hwnd, L"Icon updated! Remember to save layout.", L"Success", MB_ICONINFORMATION | MB_OK);
                        }
                    } else {
                        MessageBoxW(hwnd, L"Please select or create a layout first.", L"Warning", MB_ICONWARNING | MB_OK);
                    }
                    return 0;
                }
                
                case IDC_BTN_NEW: {
                    editor::LayoutData newLayout;
                    newLayout.name = L"New Layout";
                    newLayout.id = (int)s_layouts.size();
                    
                    wchar_t exePath[MAX_PATH] = {};
                    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                    PathRemoveFileSpecW(exePath);
                    std::wstring dir = exePath;
                    dir += L"\\Layouts\\";
                    CreateDirectoryW(dir.c_str(), nullptr);
                    newLayout.path = dir + L"New_Layout.xml";
                    
                    s_layouts.push_back(newLayout);
                    SendMessageW(s_hCombo, CB_ADDSTRING, 0, (LPARAM)newLayout.name.c_str());
                    SendMessageW(s_hCombo, CB_SETCURSEL, s_layouts.size() - 1, 0);
                    s_currentLayoutIndex = (int)s_layouts.size() - 1;
                    UpdateLayoutView(hwnd);

                    MessageBoxW(hwnd, L"New layout created! Please name it and save.", L"Keyboard Layout Editor", MB_ICONINFORMATION | MB_OK);
                    return 0;
                }

                case IDC_BTN_SAVE: {
                    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < s_layouts.size()) {
                        wchar_t newName[256];
                        GetWindowTextW(s_hEditName, newName, 256);
                        
                        s_layouts[s_currentLayoutIndex].name = newName;
                        if (s_layouts[s_currentLayoutIndex].path.empty()) {
                            wchar_t exePath[MAX_PATH] = {};
                            GetModuleFileNameW(nullptr, exePath, MAX_PATH);
                            PathRemoveFileSpecW(exePath);
                            std::wstring dir = exePath;
                            dir += L"\\Layouts\\";
                            CreateDirectoryW(dir.c_str(), nullptr);
                            s_layouts[s_currentLayoutIndex].path = dir + newName + L".xml";
                        }
                        
                        if (s_layouts[s_currentLayoutIndex].saveToFile(s_layouts[s_currentLayoutIndex].path.c_str())) {
                            SendMessageW(s_hCombo, CB_DELETESTRING, s_currentLayoutIndex, 0);
                            SendMessageW(s_hCombo, CB_INSERTSTRING, s_currentLayoutIndex, (LPARAM)newName);
                            SendMessageW(s_hCombo, CB_SETCURSEL, s_currentLayoutIndex, 0);
                            
                            MessageBoxW(hwnd, L"Layout explicitely saved back to XML!", L"Success", MB_ICONINFORMATION | MB_OK);
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
