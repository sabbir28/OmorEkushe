#include "platform/windows/main_window/main_window_types.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_background.h"
#include "platform/windows/main_window/main_window_layout.h"
#include "platform/windows/main_window/main_window_tray.h"
#include "platform/windows/main_window.h"
#include "options_overlay.h"

#include "core/app_state.h"
#include "core/layout_discovery.h"
#include "core/startup_options.h"
#include "core/window_layout_binding.h"
#include "platform/windows/resource.h"

#include <algorithm>
#include <commctrl.h>
#include <cstring>
#include <shellapi.h>
#include <string>
#include <vector>
#include <windows.h>
#include <windowsx.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "msimg32.lib") // For AlphaBlend
#endif

namespace bijoy::platform::windows {

HWND g_mainWindow = nullptr;

namespace {

HWND g_optionsButton = nullptr;
HWND g_layoutEditorButton = nullptr;
HWND g_minimizeButton = nullptr;
HWND g_closeButton = nullptr;
HWND g_tooltipWindow = nullptr;

HBITMAP g_optionsBitmap = nullptr;
HBITMAP g_layoutEditorBitmap = nullptr;

HICON g_windowClassIconLarge = nullptr;
HICON g_windowClassIconSmall = nullptr;
bool g_ownsDefaultIcon = false;

void AddTooltip(HWND tool, const wchar_t* text) {
  if (!g_tooltipWindow || !tool || !text) {
    return;
  }

  TOOLINFOW toolInfo = {};
  toolInfo.cbSize = sizeof(toolInfo);
  toolInfo.uFlags = TTF_SUBCLASS;
  toolInfo.hwnd = g_mainWindow;
  toolInfo.uId = reinterpret_cast<UINT_PTR>(tool);
  toolInfo.lpszText = const_cast<LPWSTR>(text);
  toolInfo.rect = {};

  SendMessageW(g_tooltipWindow, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&toolInfo));
}

struct ButtonState {
    bool hovered = false;
    bool pressed = false;
    HBITMAP icon = nullptr;
};

LRESULT CALLBACK IconButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR subclassId, DWORD_PTR refData) {
    auto* state = reinterpret_cast<ButtonState*>(refData);

    switch (msg) {
        case WM_MOUSEMOVE: {
            if (!state->hovered) {
                state->hovered = true;
                TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
                TrackMouseEvent(&tme);
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        }
        case WM_MOUSELEAVE: {
            state->hovered = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
        case WM_LBUTTONDOWN: {
            state->pressed = true;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
        case WM_LBUTTONUP: {
            state->pressed = false;
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);

            // Create double buffer
            HDC memDc = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HGDIOBJ oldBmp = SelectObject(memDc, memBmp);

            // 1. Draw parent background first to ensure perfect blending
            // We'll use a subtle highlight if hovered to make black icons visible.
            if (state->hovered || state->pressed) {
                HBRUSH hBrush = CreateSolidBrush(state->pressed ? RGB(180, 180, 180) : RGB(220, 220, 220));
                FillRect(memDc, &rect, hBrush);
                DeleteObject(hBrush);
            } else {
                // To blend with the AlphaBlended parent, we should ideally not fill at all 
                // or fill with a color that matches the expected background.
                // Since the window itself is AlphaBlended, we'll use a very light gray 
                // to ensure the black icon is visible even when not hovered.
                HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
                FillRect(memDc, &rect, hBrush);
                DeleteObject(hBrush);
            }

            // 2. Draw the icon with AlphaBlend
            if (state->icon) {
                BITMAP bmp;
                GetObject(state->icon, sizeof(bmp), &bmp);
                
                int iconW = 20;
                int iconH = 20;
                int x = (rect.right - iconW) / 2;
                int y = (rect.bottom - iconH) / 2;

                HDC iconDc = CreateCompatibleDC(hdc);
                HGDIOBJ oldIconBmp = SelectObject(iconDc, state->icon);

                BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
                AlphaBlend(memDc, x, y, iconW, iconH, iconDc, 0, 0, bmp.bmWidth, bmp.bmHeight, bf);

                SelectObject(iconDc, oldIconBmp);
                DeleteDC(iconDc);
            }

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDc, 0, 0, SRCCOPY);

            SelectObject(memDc, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_NCDESTROY:
            delete state;
            RemoveWindowSubclass(hwnd, IconButtonSubclassProc, subclassId);
            return 0;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE: {
      g_defaultIcon = LoadAppIconFromData(16, 16);
      g_ownsDefaultIcon = g_defaultIcon != nullptr;
      if (!g_defaultIcon) {
        g_defaultIcon = LoadIconW(nullptr, IDI_APPLICATION);
      }
      g_layoutDisplayIcon = g_defaultIcon;

      g_layoutIcon = CreateWindowExW(
          0,
          L"STATIC",
          nullptr,
          WS_CHILD | WS_VISIBLE | SS_ICON,
          10,
          9,
          16,
          16,
          hwnd,
          reinterpret_cast<HMENU>(IDC_LAYOUT_ICON),
          nullptr,
          nullptr);
      SendMessageW(g_layoutIcon, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(g_defaultIcon));

      g_layoutCombo = CreateWindowExW(
          0,
          WC_COMBOBOXW,
          nullptr,
          WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
          34,
          5,
          180,
          300,
          hwnd,
          reinterpret_cast<HMENU>(IDC_LAYOUT_COMBO),
          nullptr,
          nullptr);

      const std::wstring appDir = bijoy::core::GetAppDirectory();
      
      const std::wstring optionsCandidates[] = {
          BuildPath(appDir, L"data\\Options.png"),
          BuildPath(appDir, L"..\\data\\Options.png"),
          BuildPath(appDir, L"Options.png"),
          BuildPath(appDir, L"..\\Options.png")
      };
      for (const auto& path : optionsCandidates) {
          g_optionsBitmap = LoadPngAsBitmap(path, 20, 20);
          if (g_optionsBitmap) break;
      }

      if (!g_optionsBitmap) {
          // Diagnostic fallback or logging could go here
          // MessageBoxW(hwnd, L"Failed to load Options.png", L"Omor Ekushe Debug", MB_OK);
      }

      const std::wstring editorCandidates[] = {
          BuildPath(appDir, L"data\\LayoutEditor.png"),
          BuildPath(appDir, L"..\\data\\LayoutEditor.png"),
          BuildPath(appDir, L"LayoutEditor.png"),
          BuildPath(appDir, L"..\\LayoutEditor.png")
      };
      for (const auto& path : editorCandidates) {
          g_layoutEditorBitmap = LoadPngAsBitmap(path, 20, 20);
          if (g_layoutEditorBitmap) break;
      }

      if (!g_layoutEditorBitmap) {
          // Diagnostic fallback or logging could go here
          // MessageBoxW(hwnd, L"Failed to load LayoutEditor.png", L"Omor Ekushe Debug", MB_OK);
      }

      if (!g_optionsBitmap && !g_layoutEditorBitmap) {
          MessageBoxW(hwnd, L"Warning: Main window icons could not be loaded. Please check if 'data' folder exists with PNG files.", L"Omor Ekushe", MB_OK | MB_ICONWARNING);
      }

      g_optionsButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          nullptr,
          WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
          220,
          5,
          28,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_OPTIONS_BUTTON),
          nullptr,
          nullptr);
      if (g_optionsBitmap) {
        ButtonState* state = new ButtonState();
        state->icon = g_optionsBitmap;
        SetWindowSubclass(g_optionsButton, IconButtonSubclassProc, 1, reinterpret_cast<DWORD_PTR>(state));
      }

      g_layoutEditorButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          nullptr,
          WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
          252,
          5,
          28,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_LAYOUT_EDITOR_BUTTON),
          nullptr,
          nullptr);
      if (g_layoutEditorBitmap) {
        ButtonState* state = new ButtonState();
        state->icon = g_layoutEditorBitmap;
        SetWindowSubclass(g_layoutEditorButton, IconButtonSubclassProc, 1, reinterpret_cast<DWORD_PTR>(state));
      }

      g_minimizeButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          L"_",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          284,
          5,
          24,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_MINIMIZE_BUTTON),
          nullptr,
          nullptr);

      g_closeButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          L"X",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          312,
          5,
          24,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_CLOSE_BUTTON),
          nullptr,
          nullptr);

      g_tooltipWindow = CreateWindowExW(
          WS_EX_TOPMOST,
          TOOLTIPS_CLASSW,
          nullptr,
          WS_POPUP | TTS_ALWAYSTIP,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          hwnd,
          nullptr,
          nullptr,
          nullptr);

      SendMessageW(g_tooltipWindow, TTM_SETMAXTIPWIDTH, 0, 300);
      SendMessageW(g_tooltipWindow, TTM_SETDELAYTIME, TTDT_INITIAL, 350);
      AddTooltip(g_layoutCombo, L"Select keyboard layout");
      AddTooltip(g_optionsButton, L"Open Options dialog");
      AddTooltip(g_layoutEditorButton, L"Open Layout Editor");
      AddTooltip(g_minimizeButton, L"Minimize to system tray");
      AddTooltip(g_closeButton, L"Close Omor Ekushe");

      g_notifyIcon.cbSize = sizeof(g_notifyIcon);
      g_notifyIcon.hWnd = hwnd;
      g_notifyIcon.uID = ID_TRAY;
      g_notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
      g_notifyIcon.uCallbackMessage = kTrayIconMessage;
      g_notifyIcon.hIcon = g_defaultIcon;
      wcscpy_s(g_notifyIcon.szTip, L"Omor Ekushe");
      Shell_NotifyIconW(NIM_ADD, &g_notifyIcon);

      BuildTrayMenu();
      PopulateLayoutCombo();
      LoadBackgroundBitmapForWindow(hwnd);
      return 0;
    }

    case WM_COMMAND: {
      const UINT controlId = LOWORD(wParam);
      const UINT notifyCode = HIWORD(wParam);

      if (controlId == IDC_LAYOUT_COMBO && notifyCode == CBN_SELCHANGE) {
        const int selected = static_cast<int>(SendMessageW(g_layoutCombo, CB_GETCURSEL, 0, 0));
        if (selected >= 0) {
          const int layoutIndex = static_cast<int>(SendMessageW(g_layoutCombo, CB_GETITEMDATA, static_cast<WPARAM>(selected), 0));
          SelectLayout(layoutIndex);
        }
        return 0;
      }

      if (controlId == IDC_OPTIONS_BUTTON) {
        RECT rect;
        GetWindowRect(g_optionsButton, &rect);
        ShowOptionsOverlay(hwnd, rect.left, rect.bottom);
        return 0;
      }

      if (controlId == IDM_ABOUT) {
        MessageBoxW(hwnd, 
          L"Omor Ekushe\n"
          L"Version 1.0.0\n\n"
          L"A modern Unicode keyboard for Bangla.\n"
          L"Developed by Sabbir Legacy.", 
          L"About Omor Ekushe", MB_OK | MB_ICONINFORMATION);
        return 0;
      }

      if (controlId == IDM_LICENSE) {
        const wchar_t* licenseText = 
          L"Sabbir Legacy License (MIT)\n\n"
          L"Copyright (c) 2026 Sabbir Legacy Contributors\n\n"
          L"Permission is hereby granted, free of charge, to any person obtaining a copy "
          L"of this software and associated documentation files (the \"Software\"), to deal "
          L"in the Software without restriction, including without limitation the rights "
          L"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
          L"copies of the Software, and to permit persons to whom the Software is "
          L"furnished to do so, subject to the following conditions:\n\n"
          L"The above copyright notice and this permission notice shall be included in all "
          L"copies or substantial portions of the Software.\n\n"
          L"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
          L"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
          L"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.";
        
        MessageBoxW(hwnd, licenseText, L"Omor Ekushe License", MB_OK | MB_ICONINFORMATION);
        return 0;
      }

      if (controlId == IDM_WEBSITE) {
        ShellExecuteW(nullptr, L"open", L"https://github.com/sabbir28/OmorEkushe", nullptr, nullptr, SW_SHOWNORMAL);
        return 0;
      }

      if (controlId == IDC_LAYOUT_EDITOR_BUTTON) {
        const std::wstring appDir = bijoy::core::GetAppDirectory();
        const std::wstring editorPath = BuildPath(appDir, L"LayoutEditor.exe");

        HINSTANCE result = ShellExecuteW(hwnd, L"open", editorPath.c_str(), nullptr, nullptr, SW_SHOW);
        if (reinterpret_cast<INT_PTR>(result) <= 32) {
          MessageBoxW(hwnd, L"Failed to open Layout Editor. Ensure LayoutEditor.exe is in the application folder.", L"Omor Ekushe", MB_OK | MB_ICONERROR);
        }
        return 0;
      }

      if (controlId == IDC_MINIMIZE_BUTTON) {
        ShowWindow(hwnd, SW_HIDE);
        return 0;
      }

      if (controlId == IDC_CLOSE_BUTTON) {
        DestroyWindow(hwnd);
        return 0;
      }

      break;
    }

    case kTrayIconMessage:
      if (lParam == WM_RBUTTONUP) {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);

        BuildTrayMenu(); // Update Show/Hide text based on current window state

        const int command = TrackPopupMenu(
            g_trayMenu,
            TPM_RETURNCMD | TPM_NONBLOCKING,
            pt.x,
            pt.y,
            0,
            hwnd,
            nullptr);

        if (command) {
          OnTrayCommand(static_cast<UINT>(command));
        }
      } else if (lParam == WM_LBUTTONUP) {
        CycleToNextLayout();
      } else if (lParam == WM_LBUTTONDBLCLK) {
        ShowWindow(hwnd, SW_SHOW);
      }
      return 0;

    case WM_LBUTTONDOWN: {
      const POINT cursorPoint = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      HWND childAtPoint = ChildWindowFromPointEx(hwnd, cursorPoint, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
      if (!childAtPoint || childAtPoint == hwnd) {
        ReleaseCapture();
        SendMessageW(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        return 0;
      }
      break;
    }

    case WM_WINDOWPOSCHANGING: {
      auto* windowPos = reinterpret_cast<WINDOWPOS*>(lParam);
      if (windowPos != nullptr && (windowPos->flags & SWP_NOMOVE) == 0) {
        if (windowPos->y > 1) {
          windowPos->y = kForcedTopY;
        }
      }
      break;
    }

    case WM_EXITSIZEMOVE:
      SnapWindowToTop(hwnd);
      return 0;

    case WM_SIZE:
      if (wParam != SIZE_MINIMIZED) {
        LoadBackgroundBitmapForWindow(hwnd);
      }
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps = {};
      HDC paintDc = BeginPaint(hwnd, &ps);
      DrawWindowBackground(hwnd, paintDc);
      EndPaint(hwnd, &ps);
      return 0;
    }

    case WM_CLOSE:
      ShowWindow(hwnd, SW_HIDE);
      return 0;

    case WM_DESTROY: {
      RECT windowRect = {};
      if (GetWindowRect(hwnd, &windowRect)) {
        bijoy::core::StartupOptions options = bijoy::core::LoadStartupOptions();
        options.mainWindowLeft = windowRect.left;
        options.mainWindowTop = windowRect.top;
        options.defaultLayout = bijoy::core::GetCurrentLayoutIndex();
        bijoy::core::SaveStartupOptions(options);
      }

      Shell_NotifyIconW(NIM_DELETE, &g_notifyIcon);
      if (g_trayMenu) {
        DestroyMenu(g_trayMenu);
        g_trayMenu = nullptr;
      }
      ReleaseLayoutDisplayIcon();
      if (g_optionsBitmap) {
        DeleteObject(g_optionsBitmap);
        g_optionsBitmap = nullptr;
      }
      if (g_layoutEditorBitmap) {
        DeleteObject(g_layoutEditorBitmap);
        g_layoutEditorBitmap = nullptr;
      }
      if (g_ownsDefaultIcon && g_defaultIcon) {
        DestroyIcon(g_defaultIcon);
      }
      g_defaultIcon = nullptr;
      g_ownsDefaultIcon = false;
      if (g_windowClassIconLarge) {
        DestroyIcon(g_windowClassIconLarge);
        g_windowClassIconLarge = nullptr;
      }
      if (g_windowClassIconSmall) {
        DestroyIcon(g_windowClassIconSmall);
        g_windowClassIconSmall = nullptr;
      }
      ReleaseBackgroundBitmap();
      PostQuitMessage(0);
      return 0;
    }

    default:
      return DefWindowProcW(hwnd, msg, wParam, lParam);
  }

  return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

HWND CreateMainWindow(HINSTANCE hInstance) {
  g_windowClassIconLarge = LoadAppIconFromData(32, 32);
  g_windowClassIconSmall = LoadAppIconFromData(16, 16);

  WNDCLASSEXW windowClass = {};
  windowClass.cbSize = sizeof(windowClass);
  windowClass.lpfnWndProc = MainWindowProc;
  windowClass.hInstance = hInstance;
  windowClass.lpszClassName = L"OmorEkusheMain";
  windowClass.hIcon = g_windowClassIconLarge;
  windowClass.hIconSm = g_windowClassIconSmall;
  windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);

  RegisterClassExW(&windowClass);

  bijoy::core::StartupOptions options = bijoy::core::LoadStartupOptions();

  g_mainWindow = CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
      windowClass.lpszClassName,
      L"Omor Ekushe",
      WS_POPUP,
      options.mainWindowLeft,
      kForcedTopY,
      344,
      34,
      nullptr,
      nullptr,
      hInstance,
      nullptr);

  if (g_mainWindow) {
    ShowWindow(g_mainWindow, SW_SHOW);
    UpdateWindow(g_mainWindow);
  }

  return g_mainWindow;
}

void SetMainWindowInitialPosition(int left, int top) {
    if (g_mainWindow) {
        (void)top; // Suppress unused parameter warning as Y is forced to top
        SetWindowPos(g_mainWindow, nullptr, left, kForcedTopY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

} // namespace bijoy::platform::windows
