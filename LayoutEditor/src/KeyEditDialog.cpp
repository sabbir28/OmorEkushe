#include "KeyEditDialog.h"

namespace editor {

// Dialog Control IDs dynamically created
constexpr int IDC_LBL_NORMAL        = 3001;
constexpr int IDC_EDIT_NORMAL       = 3002;
constexpr int IDC_LBL_SHIFT         = 3003;
constexpr int IDC_EDIT_SHIFT        = 3004;
constexpr int IDC_LBL_NORMAL_OPT    = 3005;
constexpr int IDC_EDIT_NORMAL_OPT   = 3006;
constexpr int IDC_LBL_SHIFT_OPT     = 3007;
constexpr int IDC_EDIT_SHIFT_OPT    = 3008;

bool KeyEditDialog::Show(HWND parent, HINSTANCE hInstance, int keyCode, LayoutData* layout) {
    DialogData data = { keyCode, layout };

    // We can use an in-memory DLGTEMPLATE to avoid needing an .rc file for a simple project
    // But DLGTEMPLATE in memory is verbose. Since we just create a floating window, we can 
    // register a custom window class that acts like a dialog.
    
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = (WNDPROC)DialogProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"KeyEditDialogClass";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    HWND hwndDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"KeyEditDialogClass",
        L"Edit Key Mapping",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 250,
        parent,
        nullptr,
        hInstance,
        &data
    );

    if (!hwndDlg) return false;

    // Minimal message loop for the modal-like behavior
    EnableWindow(parent, FALSE);
    
    MSG msg;
    while (IsWindow(hwndDlg) && GetMessage(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessage(hwndDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
    
    return true;
}

INT_PTR CALLBACK KeyEditDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
            DialogData* data = (DialogData*)cs->lpCreateParams;
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);

            HINSTANCE hInst = cs->hInstance;
            
            HFONT hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

            auto createLbl = [&](const wchar_t* text, int x, int y, int id) {
                HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, 100, 20, hwndDlg, (HMENU)(INT_PTR)id, hInst, nullptr);
                SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            };
            auto createEdit = [&](int x, int y, int id) {
                HWND h = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, x, y, 150, 24, hwndDlg, (HMENU)(INT_PTR)id, hInst, nullptr);
                SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
                return h;
            };

            createLbl(L"Normal:", 20, 20, IDC_LBL_NORMAL);
            HWND hNormal = createEdit(120, 18, IDC_EDIT_NORMAL);

            createLbl(L"Shift:", 20, 50, IDC_LBL_SHIFT);
            HWND hShift = createEdit(120, 48, IDC_EDIT_SHIFT);

            createLbl(L"Normal Opt:", 20, 80, IDC_LBL_NORMAL_OPT);
            HWND hNormalOpt = createEdit(120, 78, IDC_EDIT_NORMAL_OPT);

            createLbl(L"Shift Opt:", 20, 110, IDC_LBL_SHIFT_OPT);
            HWND hShiftOpt = createEdit(120, 108, IDC_EDIT_SHIFT_OPT);

            HWND btnOk = CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON, 60, 160, 80, 25, hwndDlg, (HMENU)IDOK, hInst, nullptr);
            SendMessage(btnOk, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND btnCancel = CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 150, 160, 80, 25, hwndDlg, (HMENU)IDCANCEL, hInst, nullptr);
            SendMessage(btnCancel, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Populate data
            if (data && data->layout) {
                auto it = data->layout->key.find(data->keyCode);
                if (it != data->layout->key.end()) {
                    SetWindowTextW(hNormal, it->second.normal.c_str());
                    SetWindowTextW(hShift, it->second.shift.c_str());
                    SetWindowTextW(hNormalOpt, it->second.normalOption.c_str());
                    SetWindowTextW(hShiftOpt, it->second.shiftOption.c_str());
                }
            }
            
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == IDOK) {
                DialogData* data = (DialogData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (data && data->layout) {
                    wchar_t buf[256];
                    KeyMapping& mapping = data->layout->key[data->keyCode];

                    GetDlgItemTextW(hwndDlg, IDC_EDIT_NORMAL, buf, 256);
                    mapping.normal = buf;

                    GetDlgItemTextW(hwndDlg, IDC_EDIT_SHIFT, buf, 256);
                    mapping.shift = buf;

                    GetDlgItemTextW(hwndDlg, IDC_EDIT_NORMAL_OPT, buf, 256);
                    mapping.normalOption = buf;

                    GetDlgItemTextW(hwndDlg, IDC_EDIT_SHIFT_OPT, buf, 256);
                    mapping.shiftOption = buf;
                }
                DestroyWindow(hwndDlg);
                return 0;
            } else if (wmId == IDCANCEL) {
                DestroyWindow(hwndDlg);
                return 0;
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwndDlg);
            return 0;
    }
    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

} // namespace editor
