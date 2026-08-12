// =============================================================================
// File   : registration_dialog.cpp
// Module : platform/windows (License Registration Dialog)
// Purpose: Implements a dark-themed, borderless license key input dialog.
//          The dialog presents five 4-character input fields for a 20-char
//          product key, with visual features including:
//            - Custom dark background (RGB 20,20,26) with blue border accent
//            - Segoe UI / Consolas font rendering
//            - Auto-advance between key fields on character input
//            - Full clipboard paste support (splits pasted key across fields)
//            - Validation against the stored password constant
//            - Developer bypass via environment variables or sentinel files
//
//          When bypassed (via Application::Initialize), the dialog is skipped
//          and the application launches directly.
//
//          Note: Currently bypassed per user request — see application.cpp.
// =============================================================================

#include "platform/windows/registration_dialog.h"
#include "utils/system_utils.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <algorithm>
#include <string>
#include <vector>

namespace bijoy::platform::windows {

namespace {

constexpr wchar_t kRegClassName[] = L"OmorEkusheRegistrationWin32";
constexpr int kWinWidth = 520;
constexpr int kWinHeight = 330;

constexpr UINT_PTR kEdit1Id = 101;
constexpr UINT_PTR kEdit2Id = 102;
constexpr UINT_PTR kEdit3Id = 103;
constexpr UINT_PTR kEdit4Id = 104;
constexpr UINT_PTR kEdit5Id = 105;
constexpr UINT_PTR kBtnRegisterId = 201;
constexpr UINT_PTR kBtnExitId = 202;

constexpr UINT WM_NEXT_KEY_BOX = WM_APP + 100;

struct RegistrationDialogState {
    int dialogResult = 0;
    bool validationFailed = false;
    HWND hwnd = nullptr;
    HWND editControls[5] = { nullptr };
    HWND btnRegister = nullptr;
    HWND btnExit = nullptr;
    HFONT fontTitle = nullptr;
    HFONT fontSubtitle = nullptr;
    HFONT fontInput = nullptr;
    HBRUSH bgBrush = nullptr;
    HBRUSH inputBgBrush = nullptr;
};

void PopulatePastedKey(RegistrationDialogState* state, const std::wstring& rawKey) {
    if (!state) return;
    // Strip hyphens, spaces, and non-alphanumeric chars
    std::wstring cleanKey;
    for (wchar_t c : rawKey) {
        if ((c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z')) {
            cleanKey.push_back(static_cast<wchar_t>(towupper(c)));
        }
    }

    if (!cleanKey.empty()) {
        for (int i = 0; i < 5; ++i) {
            if (i * 4 < static_cast<int>(cleanKey.size())) {
                std::wstring chunk = cleanKey.substr(i * 4, 4);
                SetWindowTextW(state->editControls[i], chunk.c_str());
            }
        }
        if (cleanKey.size() >= 20 && state->btnRegister) {
            SetFocus(state->btnRegister);
        } else {
            int targetIdx = (std::min)(4, static_cast<int>(cleanKey.size() / 4));
            if (state->editControls[targetIdx]) {
                SetFocus(state->editControls[targetIdx]);
                SendMessageW(state->editControls[targetIdx], EM_SETSEL, 0, -1);
            }
        }
    }
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    auto* state = reinterpret_cast<RegistrationDialogState*>(dwRefData);
    if (!state) {
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if (msg == WM_NCDESTROY) {
        RemoveWindowSubclass(hwnd, EditSubclassProc, uIdSubclass);
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    int currentIdx = static_cast<int>(uIdSubclass - kEdit1Id);
    if (currentIdx < 0 || currentIdx >= 5) {
        return DefSubclassProc(hwnd, msg, wParam, lParam);
    }

    if (msg == WM_PASTE) {
        if (OpenClipboard(hwnd)) {
            HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            if (hData) {
                wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
                if (pText) {
                    std::wstring pastedText(pText);
                    GlobalUnlock(hData);
                    CloseClipboard();

                    PopulatePastedKey(state, pastedText);
                    return 0;
                }
            } else {
                CloseClipboard();
            }
        }
    }

    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) {
            SendMessageW(state->hwnd, WM_COMMAND, MAKEWPARAM(kBtnRegisterId, BN_CLICKED), reinterpret_cast<LPARAM>(state->btnRegister));
            return 0;
        }

        if (wParam == VK_BACK) {
            if (GetWindowTextLengthW(hwnd) == 0 && currentIdx > 0 && state->editControls[currentIdx - 1]) {
                SetFocus(state->editControls[currentIdx - 1]);
                SendMessageW(state->editControls[currentIdx - 1], EM_SETSEL, 0, -1);
                return 0;
            }
        }
    }

    if (msg == WM_CHAR) {
        if (wParam >= 'a' && wParam <= 'z') {
            wParam = wParam - 'a' + 'A';
        }
    }

    LRESULT result = DefSubclassProc(hwnd, msg, wParam, lParam);

    if (msg == WM_CHAR && wParam >= 32) {
        int textLen = GetWindowTextLengthW(hwnd);
        if (textLen >= 4) {
            PostMessageW(state->hwnd, WM_NEXT_KEY_BOX, static_cast<WPARAM>(currentIdx), 0);
        }
    }

    return result;
}

LRESULT CALLBACK RegWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<RegistrationDialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        state = static_cast<RegistrationDialogState*>(cs->lpCreateParams);
        state->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return TRUE;
    }

    if (!state) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    switch (msg) {
        case WM_CREATE: {
            state->bgBrush = CreateSolidBrush(RGB(20, 20, 26));
            state->inputBgBrush = CreateSolidBrush(RGB(38, 38, 48));

            HDC hdc = GetDC(hwnd);
            state->fontTitle = CreateFontW(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                         DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                         CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            state->fontSubtitle = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            state->fontInput = CreateFontW(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
            ReleaseDC(hwnd, hdc);

            // Create 5 input fields
            int inputWidth = 65;
            int spacing = 12;
            int totalWidth = (inputWidth * 5) + (spacing * 4);
            int startX = (kWinWidth - totalWidth) / 2;
            int startY = 135;

            for (int i = 0; i < 5; ++i) {
                HWND edit = CreateWindowExW(
                    WS_EX_CLIENTEDGE,
                    L"EDIT",
                    L"",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_CENTER | ES_UPPERCASE,
                    startX + i * (inputWidth + spacing),
                    startY,
                    inputWidth,
                    32,
                    hwnd,
                    reinterpret_cast<HMENU>(kEdit1Id + i),
                    GetModuleHandleW(nullptr),
                    nullptr
                );
                SendMessageW(edit, WM_SETFONT, reinterpret_cast<WPARAM>(state->fontInput), TRUE);
                SendMessageW(edit, EM_SETLIMITTEXT, 4, 0);
                SetWindowSubclass(edit, EditSubclassProc, kEdit1Id + i, reinterpret_cast<DWORD_PTR>(state));
                state->editControls[i] = edit;
            }

            // Register button
            state->btnRegister = CreateWindowExW(
                0,
                L"BUTTON",
                L"Register",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
                (kWinWidth - 240) / 2,
                245,
                110,
                40,
                hwnd,
                reinterpret_cast<HMENU>(kBtnRegisterId),
                GetModuleHandleW(nullptr),
                nullptr
            );
            SendMessageW(state->btnRegister, WM_SETFONT, reinterpret_cast<WPARAM>(state->fontSubtitle), TRUE);

            // Exit button
            state->btnExit = CreateWindowExW(
                0,
                L"BUTTON",
                L"Exit",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                (kWinWidth - 240) / 2 + 130,
                245,
                110,
                40,
                hwnd,
                reinterpret_cast<HMENU>(kBtnExitId),
                GetModuleHandleW(nullptr),
                nullptr
            );
            SendMessageW(state->btnExit, WM_SETFONT, reinterpret_cast<WPARAM>(state->fontSubtitle), TRUE);

            SetFocus(state->editControls[0]);
            return 0;
        }

        case WM_NEXT_KEY_BOX: {
            int currentIdx = static_cast<int>(wParam);
            if (currentIdx >= 0 && currentIdx < 4 && state->editControls[currentIdx + 1]) {
                SetFocus(state->editControls[currentIdx + 1]);
                SendMessageW(state->editControls[currentIdx + 1], EM_SETSEL, 0, -1);
            } else if (currentIdx == 4 && state->btnRegister) {
                SetFocus(state->btnRegister);
            }
            return 0;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdcStatic = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdcStatic, RGB(240, 240, 245));
            SetBkColor(hdcStatic, RGB(38, 38, 48));
            return reinterpret_cast<INT_PTR>(state->inputBgBrush);
        }

        case WM_CTLCOLORBTN: {
            return reinterpret_cast<INT_PTR>(state->bgBrush);
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);

            // Double buffering
            HDC memDc = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HGDIOBJ oldBmp = SelectObject(memDc, memBmp);

            // Background fill
            FillRect(memDc, &clientRect, state->bgBrush);

            // Outer border line
            HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(51, 153, 255));
            HGDIOBJ oldPen = SelectObject(memDc, borderPen);
            HGDIOBJ oldBrush = SelectObject(memDc, GetStockObject(NULL_BRUSH));
            Rectangle(memDc, 0, 0, clientRect.right, clientRect.bottom);
            SelectObject(memDc, oldPen);
            SelectObject(memDc, oldBrush);
            DeleteObject(borderPen);

            SetBkMode(memDc, TRANSPARENT);

            // Title
            SelectObject(memDc, state->fontTitle);
            SetTextColor(memDc, RGB(255, 255, 255));
            RECT titleRect = { 0, 30, kWinWidth, 60 };
            DrawTextW(memDc, L"Omor Ekushe Setup", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

            // Subtitle
            SelectObject(memDc, state->fontSubtitle);
            SetTextColor(memDc, RGB(180, 180, 195));
            RECT subRect = { 0, 80, kWinWidth, 110 };
            DrawTextW(memDc, L"Please enter your 20-character License Key:", -1, &subRect, DT_CENTER | DT_SINGLELINE);

            // Error text if validation failed
            if (state->validationFailed) {
                SetTextColor(memDc, RGB(255, 80, 80));
                RECT errRect = { 0, 185, kWinWidth, 215 };
                DrawTextW(memDc, L"Invalid License Key. Please try again.", -1, &errRect, DT_CENTER | DT_SINGLELINE);
            }

            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDc, 0, 0, SRCCOPY);

            SelectObject(memDc, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_COMMAND: {
            int cmdId = LOWORD(wParam);
            if (cmdId == kBtnRegisterId || (HIWORD(wParam) == BN_CLICKED && cmdId == kBtnRegisterId)) {
                std::wstring fullKey;
                for (int i = 0; i < 5; ++i) {
                    wchar_t buf[10] = {};
                    GetWindowTextW(state->editControls[i], buf, 10);
                    fullKey += buf;
                }

                if (fullKey == bijoy::utils::Password) {
                    bijoy::utils::SetRegistryPassword(bijoy::utils::Password);
                    state->dialogResult = 1;
                    DestroyWindow(hwnd);
                } else {
                    state->validationFailed = true;
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
                return 0;
            }

            if (cmdId == kBtnExitId || (HIWORD(wParam) == BN_CLICKED && cmdId == kBtnExitId)) {
                state->dialogResult = 0;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProcW(hwnd, msg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                ScreenToClient(hwnd, &pt);
                // Allow dragging from title area (top 110px)
                if (pt.y < 110) {
                    return HTCAPTION;
                }
            }
            return hit;
        }

        case WM_DESTROY: {
            if (state->fontTitle) DeleteObject(state->fontTitle);
            if (state->fontSubtitle) DeleteObject(state->fontSubtitle);
            if (state->fontInput) DeleteObject(state->fontInput);
            if (state->bgBrush) DeleteObject(state->bgBrush);
            if (state->inputBgBrush) DeleteObject(state->inputBgBrush);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

int ShowRegistrationDialog(HWND parent) {
    (void)parent;

    // Developer bypass options:
    // 1. Environment variable: OMOR_SKIP_REG=1 or SKIP_REGISTRATION=1
    // 2. Local file trigger: ".skip_registration", "skip_registration.txt", or "DEV_SKIP_REG"
    wchar_t envBuf[32] = {};
    if ((GetEnvironmentVariableW(L"OMOR_SKIP_REG", envBuf, 32) > 0 && wcscmp(envBuf, L"1") == 0) ||
        (GetEnvironmentVariableW(L"SKIP_REGISTRATION", envBuf, 32) > 0 && wcscmp(envBuf, L"1") == 0)) {
        return 1;
    }

    if (GetFileAttributesW(L".skip_registration") != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(L"skip_registration.txt") != INVALID_FILE_ATTRIBUTES ||
        GetFileAttributesW(L"DEV_SKIP_REG") != INVALID_FILE_ATTRIBUTES) {
        return 1;
    }

    // Fast-path: already registered in registry
    std::wstring savedPassword;
    bijoy::utils::GetRegistryPassword(savedPassword);

    if (savedPassword == bijoy::utils::Password) {
        return 1;
    }

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = RegWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kRegClassName;
    RegisterClassExW(&wc);

    RegistrationDialogState state;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - kWinWidth) / 2;
    int posY = (screenH - kWinHeight) / 2;

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST,
        kRegClassName,
        L"Omor Ekushe Registration",
        WS_POPUP | WS_VISIBLE,
        posX,
        posY,
        kWinWidth,
        kWinHeight,
        nullptr,
        nullptr,
        hInstance,
        &state
    );

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (IsWindow(hwnd) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_QUIT) {
            PostQuitMessage(static_cast<int>(msg.wParam));
            break;
        }
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    UnregisterClassW(kRegClassName, hInstance);
    return state.dialogResult;
}

} // namespace bijoy::platform::windows