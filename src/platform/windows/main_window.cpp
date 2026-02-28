#include "platform/windows/main_window/main_window_types.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_background.h"
#include "platform/windows/main_window/main_window_layout.h"
#include "platform/windows/main_window/main_window_tray.h"
#include "platform/windows/main_window.h"

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

      g_optionsButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          L"Options",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          220,
          5,
          60,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_OPTIONS_BUTTON),
          nullptr,
          nullptr);

      g_layoutEditorButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          L"Layout Editor",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          284,
          5,
          88,
          24,
          hwnd,
          reinterpret_cast<HMENU>(IDC_LAYOUT_EDITOR_BUTTON),
          nullptr,
          nullptr);

      g_minimizeButton = CreateWindowExW(
          0,
          WC_BUTTONW,
          L"_",
          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
          376,
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
          404,
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
        MessageBoxW(hwnd, L"Options dialog is not implemented yet.", L"Omor Ekushe", MB_OK | MB_ICONINFORMATION);
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
      436,
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
