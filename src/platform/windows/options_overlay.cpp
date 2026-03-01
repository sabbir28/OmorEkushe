#include "options_overlay.h"
#include "platform/windows/main_window/main_window_background.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/resource.h"
#include <windowsx.h>
#include <vector>
#include <string>

namespace bijoy::platform::windows {

namespace {

struct OverlayItem {
    std::wstring text;
    UINT commandId;
    RECT rect;
    bool hovered = false;
};

class OptionsOverlay {
public:
    static void Show(HWND parent, int x, int y) {
        if (s_instance) {
            SendMessage(s_instance->m_hwnd, WM_CLOSE, 0, 0);
        }
        s_instance = new OptionsOverlay(parent, x, y);
    }

private:
    OptionsOverlay(HWND parent, int x, int y) : m_parent(parent) {
        m_items.push_back({L"About", IDM_ABOUT, {0, 0, 0, 0}});
        m_items.push_back({L"License", IDM_LICENSE, {0, 0, 0, 0}});
        m_items.push_back({L"Website", IDM_WEBSITE, {0, 0, 0, 0}});

        int width = 160;
        int itemHeight = 35;
        int height = (int)m_items.size() * itemHeight + 10;

        // Pre-initialize rects to avoid issues during early messages
        for (size_t i = 0; i < m_items.size(); ++i) {
            m_items[i].rect = {5, (int)i * itemHeight + 5, width - 5, (int)(i + 1) * itemHeight + 5};
        }

        static bool registered = false;
        if (!registered) {
            WNDCLASSEXW wc = {};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.lpfnWndProc = WndProc;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.lpszClassName = L"OmorEkusheOptionsOverlay";
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            RegisterClassExW(&wc);
            registered = true;
        }

        m_hwnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            L"OmorEkusheOptionsOverlay", nullptr,
            WS_POPUP,
            x, y, width, height,
            parent, nullptr, GetModuleHandle(nullptr), this);

        if (m_hwnd) {
            ShowWindow(m_hwnd, SW_SHOW);
            SetFocus(m_hwnd);
            SetCapture(m_hwnd);
        }
    }

    ~OptionsOverlay() {
        if (s_instance == this) s_instance = nullptr;
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        OptionsOverlay* self = nullptr;
        if (msg == WM_NCCREATE) {
            self = static_cast<OptionsOverlay*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            if (self) self->m_hwnd = hwnd;
        } else {
            self = reinterpret_cast<OptionsOverlay*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self) {
            return self->HandleMessage(hwnd, msg, wParam, lParam);
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                if (!hdc) return 0;
                
                RECT clientRect;
                GetClientRect(hwnd, &clientRect);
                int w = clientRect.right;
                int h = clientRect.bottom;
                if (w <= 0 || h <= 0) {
                    EndPaint(hwnd, &ps);
                    return 0;
                }

                HDC memDc = CreateCompatibleDC(hdc);
                HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
                HGDIOBJ oldMemBmp = SelectObject(memDc, memBmp);

                // 1. Draw Background
                bool bgDrawn = false;
                if (g_backgroundBitmap) {
                    BITMAP bmp;
                    if (GetObject(g_backgroundBitmap, sizeof(bmp), &bmp) && bmp.bmWidth > 0 && bmp.bmHeight > 0) {
                        HDC bgDc = CreateCompatibleDC(hdc);
                        HGDIOBJ oldBgBmp = SelectObject(bgDc, g_backgroundBitmap);
                        StretchBlt(memDc, 0, 0, w, h, bgDc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
                        SelectObject(bgDc, oldBgBmp);
                        DeleteDC(bgDc);
                        bgDrawn = true;
                    }
                }
                
                if (bgDrawn) {
                    HDC dcDark = CreateCompatibleDC(hdc);
                    HBITMAP bmpDark = CreateCompatibleBitmap(hdc, w, h);
                    HGDIOBJ oldDarkBmp = SelectObject(dcDark, bmpDark);
                    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
                    FillRect(dcDark, &clientRect, hBrush);
                    DeleteObject(hBrush);
                    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 180, 0}; 
                    AlphaBlend(memDc, 0, 0, w, h, dcDark, 0, 0, w, h, bf);
                    SelectObject(dcDark, oldDarkBmp);
                    DeleteObject(bmpDark);
                    DeleteDC(dcDark);
                } else {
                    HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 30));
                    FillRect(memDc, &clientRect, bgBrush);
                    DeleteObject(bgBrush);
                }

                // 2. Draw Items
                HGDIOBJ oldFont = SelectObject(memDc, GetStockObject(DEFAULT_GUI_FONT));
                SetBkMode(memDc, TRANSPARENT);

                for (const auto& item : m_items) {
                    int itemW = item.rect.right - item.rect.left;
                    int itemH = item.rect.bottom - item.rect.top;
                    
                    if (item.hovered && itemW > 0 && itemH > 0) {
                        HBRUSH hHover = CreateSolidBrush(RGB(255, 255, 255));
                        HDC dcHover = CreateCompatibleDC(hdc);
                        HBITMAP bmpHover = CreateCompatibleBitmap(hdc, itemW, itemH);
                        HGDIOBJ oldHoverBmp = SelectObject(dcHover, bmpHover);
                        RECT localHoverRect = {0, 0, itemW, itemH};
                        FillRect(dcHover, &localHoverRect, hHover);
                        BLENDFUNCTION bf = {AC_SRC_OVER, 0, 40, 0};
                        AlphaBlend(memDc, item.rect.left, item.rect.top, itemW, itemH, 
                                   dcHover, 0, 0, itemW, itemH, bf);
                        SelectObject(dcHover, oldHoverBmp);
                        DeleteObject(bmpHover);
                        DeleteDC(dcHover);
                        DeleteObject(hHover);
                        SetTextColor(memDc, RGB(255, 255, 255));
                    } else {
                        SetTextColor(memDc, RGB(220, 220, 220));
                    }
                    RECT textRect = item.rect;
                    textRect.left += 15;
                    DrawTextW(memDc, item.text.c_str(), -1, &textRect, DT_SINGLELINE | DT_VCENTER);
                }

                // 3. Border
                HBRUSH borderBrush = CreateSolidBrush(RGB(100, 100, 100));
                FrameRect(memDc, &clientRect, borderBrush);
                DeleteObject(borderBrush);

                BitBlt(hdc, 0, 0, w, h, memDc, 0, 0, SRCCOPY);

                SelectObject(memDc, oldFont);
                SelectObject(memDc, oldMemBmp);
                DeleteObject(memBmp);
                DeleteDC(memDc);
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_MOUSEMOVE: {
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                bool changed = false;
                for (auto& item : m_items) {
                    bool wasHovered = item.hovered;
                    item.hovered = PtInRect(&item.rect, pt) != 0;
                    if (item.hovered != wasHovered) changed = true;
                }
                if (changed) InvalidateRect(hwnd, nullptr, FALSE);
                break;
            }
            case WM_LBUTTONDOWN: {
                POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                for (auto& item : m_items) {
                    if (PtInRect(&item.rect, pt)) {
                        HWND parent = m_parent;
                        UINT cmd = item.commandId;
                        DestroyWindow(hwnd);
                        PostMessage(parent, WM_COMMAND, MAKEWPARAM(cmd, 0), 0);
                        return 0;
                    }
                }
                DestroyWindow(hwnd);
                break;
            }
            case WM_KILLFOCUS:
                DestroyWindow(hwnd);
                break;
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            case WM_NCDESTROY:
                ReleaseCapture();
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
                delete this;
                return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    HWND m_hwnd = nullptr;
    HWND m_parent;
    std::vector<OverlayItem> m_items;
    static OptionsOverlay* s_instance;
};

OptionsOverlay* OptionsOverlay::s_instance = nullptr;

} // namespace

void ShowOptionsOverlay(HWND parent, int x, int y) {
    OptionsOverlay::Show(parent, x, y);
}

} // namespace bijoy::platform::windows
