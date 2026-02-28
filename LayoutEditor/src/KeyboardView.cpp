#include "KeyboardView.h"
#include <vector>

namespace editor {

struct KeyboardKey {
    int keyCode;
    float rx, ry, rw, rh; // Relative coordinates 0.0 to 1.0 (approximated)
    bool modifiable;
    const wchar_t* label;
};

// Full standard 104-key US keyboard approximation
static std::vector<KeyboardKey> g_BaseKeys = {
    // Esc & Function Keys (Row 0)
    {27,  0.00f, 0.00f, 0.06f, 0.15f, false, L"Esc"},
    {112, 0.10f, 0.00f, 0.06f, 0.15f, false, L"F1"}, {113, 0.16f, 0.00f, 0.06f, 0.15f, false, L"F2"}, {114, 0.22f, 0.00f, 0.06f, 0.15f, false, L"F3"}, {115, 0.28f, 0.00f, 0.06f, 0.15f, false, L"F4"},
    {116, 0.36f, 0.00f, 0.06f, 0.15f, false, L"F5"}, {117, 0.42f, 0.00f, 0.06f, 0.15f, false, L"F6"}, {118, 0.48f, 0.00f, 0.06f, 0.15f, false, L"F7"}, {119, 0.54f, 0.00f, 0.06f, 0.15f, false, L"F8"},
    {120, 0.62f, 0.00f, 0.06f, 0.15f, false, L"F9"}, {121, 0.68f, 0.00f, 0.06f, 0.15f, false, L"F10"}, {122, 0.74f, 0.00f, 0.06f, 0.15f, false, L"F11"}, {123, 0.80f, 0.00f, 0.06f, 0.15f, false, L"F12"},
    {44,  0.87f, 0.00f, 0.05f, 0.15f, false, L"Prt"}, {145, 0.93f, 0.00f, 0.05f, 0.15f, false, L"Scr"}, {19, 0.99f, 0.00f, 0.05f, 0.15f, false, L"Pse"},

    // Number Row (Row 1) (Y offset 0.18)
    {192, 0.00f, 0.18f, 0.05f, 0.15f, true, L"`"}, {49,  0.05f, 0.18f, 0.05f, 0.15f, true, L"1"}, {50,  0.10f, 0.18f, 0.05f, 0.15f, true, L"2"},
    {51,  0.15f, 0.18f, 0.05f, 0.15f, true, L"3"}, {52,  0.20f, 0.18f, 0.05f, 0.15f, true, L"4"}, {53,  0.25f, 0.18f, 0.05f, 0.15f, true, L"5"},
    {54,  0.30f, 0.18f, 0.05f, 0.15f, true, L"6"}, {55,  0.35f, 0.18f, 0.05f, 0.15f, true, L"7"}, {56,  0.40f, 0.18f, 0.05f, 0.15f, true, L"8"},
    {57,  0.45f, 0.18f, 0.05f, 0.15f, true, L"9"}, {48,  0.50f, 0.18f, 0.05f, 0.15f, true, L"0"}, {189, 0.55f, 0.18f, 0.05f, 0.15f, true, L"-"},
    {187, 0.60f, 0.18f, 0.05f, 0.15f, true, L"="}, {8,   0.65f, 0.18f, 0.10f, 0.15f, false, L"Back"},
    {45,  0.77f, 0.18f, 0.05f, 0.15f, false, L"Ins"}, {36,  0.83f, 0.18f, 0.05f, 0.15f, false, L"Hm"}, {33, 0.89f, 0.18f, 0.05f, 0.15f, false, L"Up"},

    // QWERTY Row (Row 2) (Y offset 0.36)
    {9,   0.00f, 0.36f, 0.08f, 0.15f, false, L"Tab"}, {81,  0.08f, 0.36f, 0.05f, 0.15f, true, L"Q"}, {87,  0.13f, 0.36f, 0.05f, 0.15f, true, L"W"},
    {69,  0.18f, 0.36f, 0.05f, 0.15f, true, L"E"}, {82,  0.23f, 0.36f, 0.05f, 0.15f, true, L"R"}, {84,  0.28f, 0.36f, 0.05f, 0.15f, true, L"T"},
    {89,  0.33f, 0.36f, 0.05f, 0.15f, true, L"Y"}, {85,  0.38f, 0.36f, 0.05f, 0.15f, true, L"U"}, {73,  0.43f, 0.36f, 0.05f, 0.15f, true, L"I"},
    {79,  0.48f, 0.36f, 0.05f, 0.15f, true, L"O"}, {80,  0.53f, 0.36f, 0.05f, 0.15f, true, L"P"}, {219, 0.58f, 0.36f, 0.05f, 0.15f, true, L"["},
    {221, 0.63f, 0.36f, 0.05f, 0.15f, true, L"]"}, {220, 0.68f, 0.36f, 0.07f, 0.15f, true, L"\\"},
    {46,  0.77f, 0.36f, 0.05f, 0.15f, false, L"Del"}, {35,  0.83f, 0.36f, 0.05f, 0.15f, false, L"End"}, {34, 0.89f, 0.36f, 0.05f, 0.15f, false, L"Dn"},

    // ASDF Row (Row 3) (Y offset 0.54)
    {20,  0.00f, 0.54f, 0.10f, 0.15f, false, L"Caps"}, {65,  0.10f, 0.54f, 0.05f, 0.15f, true, L"A"}, {83,  0.15f, 0.54f, 0.05f, 0.15f, true, L"S"},
    {68,  0.20f, 0.54f, 0.05f, 0.15f, true, L"D"}, {70,  0.25f, 0.54f, 0.05f, 0.15f, true, L"F"}, {71,  0.30f, 0.54f, 0.05f, 0.15f, true, L"G"},
    {72,  0.35f, 0.54f, 0.05f, 0.15f, true, L"H"}, {74,  0.40f, 0.54f, 0.05f, 0.15f, true, L"J"}, {75,  0.45f, 0.54f, 0.05f, 0.15f, true, L"K"},
    {76,  0.50f, 0.54f, 0.05f, 0.15f, true, L"L"}, {186, 0.55f, 0.54f, 0.05f, 0.15f, true, L";"}, {222, 0.60f, 0.54f, 0.05f, 0.15f, true, L"'"},
    {13,  0.65f, 0.54f, 0.10f, 0.15f, false, L"Enter"},

    // ZXCV Row (Row 4) (Y offset 0.72)
    {16,  0.00f, 0.72f, 0.13f, 0.15f, false, L"Shift"}, {90,  0.13f, 0.72f, 0.05f, 0.15f, true, L"Z"}, {88,  0.18f, 0.72f, 0.05f, 0.15f, true, L"X"},
    {67,  0.23f, 0.72f, 0.05f, 0.15f, true, L"C"}, {86,  0.28f, 0.72f, 0.05f, 0.15f, true, L"V"}, {66,  0.33f, 0.72f, 0.05f, 0.15f, true, L"B"},
    {78,  0.38f, 0.72f, 0.05f, 0.15f, true, L"N"}, {77,  0.43f, 0.72f, 0.05f, 0.15f, true, L"M"}, {188, 0.48f, 0.72f, 0.05f, 0.15f, true, L","},
    {190, 0.53f, 0.72f, 0.05f, 0.15f, true, L"."}, {191, 0.58f, 0.72f, 0.05f, 0.15f, true, L"/"}, {16, 0.63f, 0.72f, 0.12f, 0.15f, false, L"Shift"},
    {38,  0.83f, 0.72f, 0.05f, 0.15f, false, L"Up"},

    // Space Row (Row 5) (Y offset 0.90)
    {17,  0.00f, 0.90f, 0.06f, 0.10f, false, L"Ctrl"}, {91,  0.06f, 0.90f, 0.06f, 0.10f, false, L"Win"}, {18,  0.12f, 0.90f, 0.06f, 0.10f, false, L"Alt"},
    {32,  0.18f, 0.90f, 0.39f, 0.10f, false, L"Space"}, {18,  0.57f, 0.90f, 0.06f, 0.10f, false, L"Alt"}, {92,  0.63f, 0.90f, 0.06f, 0.10f, false, L"Win"},
    {93,  0.69f, 0.90f, 0.06f, 0.10f, false, L"Menu"}, {17,  0.75f, 0.90f, 0.06f, 0.10f, false, L"Ctrl"},
    {37,  0.77f, 0.90f, 0.05f, 0.10f, false, L"Left"}, {40,  0.83f, 0.90f, 0.05f, 0.10f, false, L"Dn"}, {39, 0.89f, 0.90f, 0.05f, 0.10f, false, L"Rt"}
};

bool KeyboardView::Register(HINSTANCE hInstance) {
    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"KeyboardViewControl";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    
    return RegisterClassExW(&wc) != 0;
}

HWND KeyboardView::Create(HWND parent, HINSTANCE hInstance, int x, int y, int width, int height, int id) {
    return CreateWindowExW(
        0,
        L"KeyboardViewControl",
        L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height,
        parent,
        (HMENU)(INT_PTR)id,
        hInstance,
        nullptr
    );
}

void KeyboardView::SetLayout(HWND hwnd, const LayoutData* layout) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)layout);
    InvalidateRect(hwnd, nullptr, TRUE);
}

const LayoutData* KeyboardView::GetLayout(HWND hwnd) {
    return (const LayoutData*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
}

void KeyboardView::OnPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    const LayoutData* layout = GetLayout(hwnd);

    // Draw background
    HBRUSH bgBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &rc, bgBrush);
    DeleteObject(bgBrush);

    HBRUSH keyBrush = CreateSolidBrush(RGB(255, 255, 255));
    HPEN keyPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    SelectObject(hdc, keyBrush);
    SelectObject(hdc, keyPen);

    HFONT hFontMain = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT hFontSmall = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    
    SetBkMode(hdc, TRANSPARENT);

    for (const auto& k : g_BaseKeys) {
        int kx = (int)(k.rx * w);
        int ky = (int)(k.ry * h);
        int kw = (int)(k.rw * w);
        int kh = (int)(k.rh * h);

        RECT krc = { kx + 2, ky + 2, kx + kw - 2, ky + kh - 2 };
        Rectangle(hdc, krc.left, krc.top, krc.right, krc.bottom);

        // Draw faint base label first
        if (k.modifiable) {
            SelectObject(hdc, hFontMain);
            SetTextColor(hdc, RGB(220, 220, 220)); // Faint gray
            RECT bgRect = krc;
            bgRect.top += 5; // Slight offset from top
            DrawTextW(hdc, k.label, -1, &bgRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        // Draw mapped layoyt characters
        if (k.modifiable) {
            std::wstring normalText = L"";
            std::wstring shiftText = L"";

            if (layout) {
                auto it = layout->key.find(k.keyCode);
                if (it != layout->key.end()) {
                    normalText = it->second.normal;
                    shiftText = it->second.shift;
                }
            }

            SetTextColor(hdc, RGB(0, 0, 0));
            SelectObject(hdc, hFontMain);
            
            // Draw normal character bottom left
            if (!normalText.empty()) {
                RECT tRectNormal = krc;
                tRectNormal.left += 5;
                tRectNormal.top += 15;
                DrawTextW(hdc, normalText.c_str(), -1, &tRectNormal, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }

            // Draw shift character top left
            if (!shiftText.empty()) {
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(100, 100, 100));
                RECT tRectShift = krc;
                tRectShift.left += 5;
                tRectShift.top += 2;
                DrawTextW(hdc, shiftText.c_str(), -1, &tRectShift, DT_LEFT | DT_TOP | DT_SINGLELINE);
            }
        } else {
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, RGB(50, 50, 50));
            DrawTextW(hdc, k.label, -1, &krc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }

    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    DeleteObject(hFontMain);
    DeleteObject(hFontSmall);
    DeleteObject(keyBrush);
    DeleteObject(keyPen);
    
    EndPaint(hwnd, &ps);
}

void KeyboardView::OnLButtonDown(HWND hwnd, int x, int y) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    for (const auto& k : g_BaseKeys) {
        int kx = (int)(k.rx * w);
        int ky = (int)(k.ry * h);
        int kw = (int)(k.rw * w);
        int kh = (int)(k.rh * h);

        if (x >= kx && x <= kx + kw && y >= ky && y <= ky + kh) {
            // Found clicked key.
            if (k.modifiable) {
                // Notify parent window.
                SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), k.keyCode), (LPARAM)hwnd);
            }
            break;
        }
    }
}

LRESULT CALLBACK KeyboardView::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            OnPaint(hwnd);
            return 0;
        case WM_LBUTTONDOWN:
            OnLButtonDown(hwnd, LOWORD(lParam), HIWORD(lParam));
            return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

} // namespace editor
