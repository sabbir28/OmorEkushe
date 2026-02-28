#include "KeyboardView.h"
#include <vector>

namespace editor {

struct KeyboardKey {
    int keyCode;
    float rx, ry, rw, rh; // Relative coordinates 0.0 to 1.0 (approximated)
    const wchar_t* normalChar;
    const wchar_t* shiftChar;
};

// Very basic and simplified relative layout for main keyboard block
static std::vector<KeyboardKey> g_BaseKeys = {
    // Top row (digits)
    {192, 0.00f, 0.0f, 0.06f, 0.2f, L"", L""}, {49,  0.06f, 0.0f, 0.06f, 0.2f, L"", L""}, {50,  0.12f, 0.0f, 0.06f, 0.2f, L"", L""},
    {51,  0.18f, 0.0f, 0.06f, 0.2f, L"", L""}, {52,  0.24f, 0.0f, 0.06f, 0.2f, L"", L""}, {53,  0.30f, 0.0f, 0.06f, 0.2f, L"", L""},
    {54,  0.36f, 0.0f, 0.06f, 0.2f, L"", L""}, {55,  0.42f, 0.0f, 0.06f, 0.2f, L"", L""}, {56,  0.48f, 0.0f, 0.06f, 0.2f, L"", L""},
    {57,  0.54f, 0.0f, 0.06f, 0.2f, L"", L""}, {48,  0.60f, 0.0f, 0.06f, 0.2f, L"", L""}, {189, 0.66f, 0.0f, 0.06f, 0.2f, L"", L""},
    {187, 0.72f, 0.0f, 0.06f, 0.2f, L"", L""},

    // Row 1
    {81,  0.08f, 0.2f, 0.06f, 0.2f, L"", L""}, {87,  0.14f, 0.2f, 0.06f, 0.2f, L"", L""}, {69,  0.20f, 0.2f, 0.06f, 0.2f, L"", L""},
    {82,  0.26f, 0.2f, 0.06f, 0.2f, L"", L""}, {84,  0.32f, 0.2f, 0.06f, 0.2f, L"", L""}, {89,  0.38f, 0.2f, 0.06f, 0.2f, L"", L""},
    {85,  0.44f, 0.2f, 0.06f, 0.2f, L"", L""}, {73,  0.50f, 0.2f, 0.06f, 0.2f, L"", L""}, {79,  0.56f, 0.2f, 0.06f, 0.2f, L"", L""},
    {80,  0.62f, 0.2f, 0.06f, 0.2f, L"", L""}, {219, 0.68f, 0.2f, 0.06f, 0.2f, L"", L""}, {221, 0.74f, 0.2f, 0.06f, 0.2f, L"", L""},

    // Row 2
    {65,  0.10f, 0.4f, 0.06f, 0.2f, L"", L""}, {83,  0.16f, 0.4f, 0.06f, 0.2f, L"", L""}, {68,  0.22f, 0.4f, 0.06f, 0.2f, L"", L""},
    {70,  0.28f, 0.4f, 0.06f, 0.2f, L"", L""}, {71,  0.34f, 0.4f, 0.06f, 0.2f, L"", L""}, {72,  0.40f, 0.4f, 0.06f, 0.2f, L"", L""},
    {74,  0.46f, 0.4f, 0.06f, 0.2f, L"", L""}, {75,  0.52f, 0.4f, 0.06f, 0.2f, L"", L""}, {76,  0.58f, 0.4f, 0.06f, 0.2f, L"", L""},
    {186, 0.64f, 0.4f, 0.06f, 0.2f, L"", L""}, {222, 0.70f, 0.4f, 0.06f, 0.2f, L"", L""}, {220, 0.76f, 0.4f, 0.06f, 0.2f, L"", L""},

    // Row 3
    {90,  0.12f, 0.6f, 0.06f, 0.2f, L"", L""}, {88,  0.18f, 0.6f, 0.06f, 0.2f, L"", L""}, {67,  0.24f, 0.6f, 0.06f, 0.2f, L"", L""},
    {86,  0.30f, 0.6f, 0.06f, 0.2f, L"", L""}, {66,  0.36f, 0.6f, 0.06f, 0.2f, L"", L""}, {78,  0.42f, 0.6f, 0.06f, 0.2f, L"", L""},
    {77,  0.48f, 0.6f, 0.06f, 0.2f, L"", L""}, {188, 0.54f, 0.6f, 0.06f, 0.2f, L"", L""}, {190, 0.60f, 0.6f, 0.06f, 0.2f, L"", L""},
    {191, 0.66f, 0.6f, 0.06f, 0.2f, L"", L""}
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

        // Draw mapped layoyt characters
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
        RECT tRectNormal = krc;
        tRectNormal.left += 5;
        tRectNormal.top += 15;
        DrawTextW(hdc, normalText.c_str(), -1, &tRectNormal, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Draw shift character top left
        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, RGB(100, 100, 100));
        RECT tRectShift = krc;
        tRectShift.left += 5;
        tRectShift.top += 2;
        DrawTextW(hdc, shiftText.c_str(), -1, &tRectShift, DT_LEFT | DT_TOP | DT_SINGLELINE);
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
            // Found clicked key. Notify parent window.
            SendMessageW(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), k.keyCode), (LPARAM)hwnd);
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
