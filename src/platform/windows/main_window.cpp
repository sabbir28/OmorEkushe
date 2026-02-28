#include "platform/windows/main_window.h"

#include "core/app_state.h"
#include "core/layout_discovery.h"
#include "core/startup_options.h"
#include "core/window_layout_binding.h"
#include "platform/windows/resource.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

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
#endif

#ifndef TPM_NONBLOCKING
#define TPM_NONBLOCKING 0x0000L
#endif

namespace bijoy::platform::windows {

namespace {

constexpr UINT kTrayIconMessage = WM_USER + 1;
HWND g_mainWindow = nullptr;
HWND g_layoutCombo = nullptr;
HWND g_layoutIcon = nullptr;
HWND g_optionsButton = nullptr;
HWND g_layoutEditorButton = nullptr;
HWND g_minimizeButton = nullptr;
HWND g_closeButton = nullptr;
HWND g_tooltipWindow = nullptr;

NOTIFYICONDATAW g_notifyIcon = {};
HMENU g_trayMenu = nullptr;
HICON g_defaultIcon = nullptr;
HICON g_layoutDisplayIcon = nullptr;
HICON g_windowClassIconLarge = nullptr;
HICON g_windowClassIconSmall = nullptr;
bool g_ownsDefaultIcon = false;
int g_requestedWindowTop = 0;
HBITMAP g_backgroundBitmap = nullptr;
int g_backgroundOpacity = 72;

constexpr int kForcedTopY = 0;

void SnapWindowToTop(HWND hwnd) {
  if (!hwnd) {
    return;
  }

  RECT windowRect = {};
  if (!GetWindowRect(hwnd, &windowRect)) {
    return;
  }

  SetWindowPos(
      hwnd,
      nullptr,
      windowRect.left,
      kForcedTopY,
      0,
      0,
      SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


std::wstring BuildPath(const std::wstring& base, const wchar_t* relative) {
  std::wstring path = base;
  path += relative;
  return path;
}

std::wstring BuildPath(const std::wstring& base, const std::wstring& relative) {
  std::wstring path = base;
  path += relative;
  return path;
}

HICON LoadIconFromPath(const std::wstring& path, int width, int height) {
  return static_cast<HICON>(LoadImageW(
      nullptr,
      path.c_str(),
      IMAGE_ICON,
      width,
      height,
      LR_LOADFROMFILE));
}

HICON LoadAppIconFromData(int width, int height) {
  const std::wstring appDir = bijoy::core::GetAppDirectory();
  const std::wstring candidates[] = {
      BuildPath(appDir, L"data\\Icons\\Bijoy.ico"),
      BuildPath(appDir, L"..\\data\\Icons\\Bijoy.ico"),
      BuildPath(appDir, L"data\\Bijoy.ico"),
      BuildPath(appDir, L"..\\data\\Bijoy.ico"),
      BuildPath(appDir, L"Bijoy.ico")};

  for (const auto& candidate : candidates) {
    if (HICON icon = LoadIconFromPath(candidate, width, height)) {
      return icon;
    }
  }

  return nullptr;
}

HICON LoadLayoutIcon(const bijoy::core::Layout* layout) {
  if (!layout || layout->iconName.empty()) {
    return nullptr;
  }

  std::wstring iconName = layout->iconName;
  const bool hasIcoExt =
      iconName.size() > 4 &&
      _wcsicmp(iconName.c_str() + iconName.size() - 4, L".ico") == 0;
  if (!hasIcoExt) {
    iconName += L".ico";
  }

  const std::wstring appDir = bijoy::core::GetAppDirectory();
  const std::wstring iconPaths[] = {
      BuildPath(appDir, std::wstring(L"Icons\\") + iconName),
      BuildPath(appDir, std::wstring(L"..\\data\\Icons\\") + iconName),
  };

  for (const auto& iconPath : iconPaths) {
    if (HICON icon = LoadIconFromPath(iconPath, 16, 16)) {
      return icon;
    }
  }

  return nullptr;
}



std::string WideToUtf8(const std::wstring& value) {
  if (value.empty()) {
    return {};
  }

  const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (requiredSize <= 0) {
    return {};
  }

  std::string utf8(static_cast<size_t>(requiredSize - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, utf8.data(), requiredSize, nullptr, nullptr);
  return utf8;
}

void ReleaseBackgroundBitmap() {
  if (g_backgroundBitmap) {
    DeleteObject(g_backgroundBitmap);
    g_backgroundBitmap = nullptr;
  }
}

std::wstring ResolveBackgroundImagePath() {
  const std::wstring appDir = bijoy::core::GetAppDirectory();
  const std::wstring candidates[] = {
      BuildPath(appDir, L"data\\Bann2011.jpg"),
      BuildPath(appDir, L"..\\data\\Bann2011.jpg"),
      BuildPath(appDir, L"Bann2011.jpg")};

  for (const auto& candidate : candidates) {
    if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
      return candidate;
    }
  }

  return L"";
}

void LoadBackgroundBitmapForWindow(HWND hwnd) {
  if (!hwnd) {
    return;
  }

  ReleaseBackgroundBitmap();

  RECT clientRect = {};
  if (!GetClientRect(hwnd, &clientRect)) {
    return;
  }

  const LONG clientWidthLong = std::max<LONG>(1, clientRect.right - clientRect.left);
  const LONG clientHeightLong = std::max<LONG>(1, clientRect.bottom - clientRect.top);
  const int clientWidth = static_cast<int>(clientWidthLong);
  const int clientHeight = static_cast<int>(clientHeightLong);

  const std::wstring imagePath = ResolveBackgroundImagePath();
  if (imagePath.empty()) {
    return;
  }

  const std::string imagePathUtf8 = WideToUtf8(imagePath);
  if (imagePathUtf8.empty()) {
    return;
  }

  int sourceWidth = 0;
  int sourceHeight = 0;
  int sourceChannels = 0;
  stbi_uc* sourcePixels = stbi_load(imagePathUtf8.c_str(), &sourceWidth, &sourceHeight, &sourceChannels, 4);
  if (!sourcePixels || sourceWidth <= 0 || sourceHeight <= 0) {
    stbi_image_free(sourcePixels);
    return;
  }

  BITMAPINFO bitmapInfo = {};
  bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmapInfo.bmiHeader.biWidth = clientWidth;
  bitmapInfo.bmiHeader.biHeight = -clientHeight;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  void* scaledPixels = nullptr;
  HDC screenDc = GetDC(hwnd);
  HBITMAP dib = CreateDIBSection(screenDc, &bitmapInfo, DIB_RGB_COLORS, &scaledPixels, nullptr, 0);

  if (!screenDc || !dib || !scaledPixels) {
    if (screenDc) {
      ReleaseDC(hwnd, screenDc);
    }
    if (dib) {
      DeleteObject(dib);
    }
    stbi_image_free(sourcePixels);
    return;
  }

  HDC srcDc = CreateCompatibleDC(screenDc);
  HDC dstDc = CreateCompatibleDC(screenDc);

  BITMAPINFO srcInfo = {};
  srcInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  srcInfo.bmiHeader.biWidth = sourceWidth;
  srcInfo.bmiHeader.biHeight = -sourceHeight;
  srcInfo.bmiHeader.biPlanes = 1;
  srcInfo.bmiHeader.biBitCount = 32;
  srcInfo.bmiHeader.biCompression = BI_RGB;

  void* srcDibPixels = nullptr;
  HBITMAP srcBitmap = CreateDIBSection(screenDc, &srcInfo, DIB_RGB_COLORS, &srcDibPixels, nullptr, 0);

  if (!srcDc || !dstDc || !srcBitmap || !srcDibPixels) {
    if (srcBitmap) {
      DeleteObject(srcBitmap);
    }
    if (srcDc) {
      DeleteDC(srcDc);
    }
    if (dstDc) {
      DeleteDC(dstDc);
    }
    DeleteObject(dib);
    ReleaseDC(hwnd, screenDc);
    stbi_image_free(sourcePixels);
    return;
  }

  std::vector<unsigned char> srcBgra(static_cast<size_t>(sourceWidth) * static_cast<size_t>(sourceHeight) * 4);
  for (int i = 0; i < sourceWidth * sourceHeight; ++i) {
    srcBgra[i * 4 + 0] = sourcePixels[i * 4 + 2];
    srcBgra[i * 4 + 1] = sourcePixels[i * 4 + 1];
    srcBgra[i * 4 + 2] = sourcePixels[i * 4 + 0];
    srcBgra[i * 4 + 3] = sourcePixels[i * 4 + 3];
  }

  std::memcpy(srcDibPixels, srcBgra.data(), srcBgra.size());

  const HGDIOBJ oldSrc = SelectObject(srcDc, srcBitmap);
  const HGDIOBJ oldDst = SelectObject(dstDc, dib);
  SetStretchBltMode(dstDc, HALFTONE);
  SetBrushOrgEx(dstDc, 0, 0, nullptr);
  StretchBlt(dstDc, 0, 0, clientWidth, clientHeight, srcDc, 0, 0, sourceWidth, sourceHeight, SRCCOPY);
  SelectObject(srcDc, oldSrc);
  SelectObject(dstDc, oldDst);

  DeleteObject(srcBitmap);
  DeleteDC(srcDc);
  DeleteDC(dstDc);
  ReleaseDC(hwnd, screenDc);
  stbi_image_free(sourcePixels);

  g_backgroundBitmap = dib;
}

void DrawWindowBackground(HWND hwnd, HDC targetDc) {
  RECT clientRect = {};
  GetClientRect(hwnd, &clientRect);

  if (!g_backgroundBitmap) {
    FillRect(targetDc, &clientRect, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
    return;
  }

  HDC memDc = CreateCompatibleDC(targetDc);
  if (!memDc) {
    FillRect(targetDc, &clientRect, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
    return;
  }

  const HGDIOBJ oldBmp = SelectObject(memDc, g_backgroundBitmap);
  BLENDFUNCTION blend = {};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = static_cast<BYTE>(g_backgroundOpacity);
  blend.AlphaFormat = 0;

  FillRect(targetDc, &clientRect, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
  AlphaBlend(
      targetDc,
      0,
      0,
      clientRect.right - clientRect.left,
      clientRect.bottom - clientRect.top,
      memDc,
      0,
      0,
      clientRect.right - clientRect.left,
      clientRect.bottom - clientRect.top,
      blend);

  SelectObject(memDc, oldBmp);
  DeleteDC(memDc);
}
void SetMainWindowVisible(bool visible) {
  if (!g_mainWindow) {
    return;
  }
  ShowWindow(g_mainWindow, visible ? SW_SHOW : SW_HIDE);
}

void ReleaseLayoutDisplayIcon() {
  if (g_layoutDisplayIcon && g_layoutDisplayIcon != g_defaultIcon) {
    DestroyIcon(g_layoutDisplayIcon);
  }
  g_layoutDisplayIcon = nullptr;
}

void SelectLayout(int index) {
  bijoy::core::SetCurrentLayout(index);

  if (g_layoutCombo) {
    const int count = static_cast<int>(SendMessageW(g_layoutCombo, CB_GETCOUNT, 0, 0));
    for (int comboIndex = 0; comboIndex < count; ++comboIndex) {
      const LRESULT itemData = SendMessageW(g_layoutCombo, CB_GETITEMDATA, static_cast<WPARAM>(comboIndex), 0);
      if (static_cast<int>(itemData) == index) {
        SendMessageW(g_layoutCombo, CB_SETCURSEL, static_cast<WPARAM>(comboIndex), 0);
        break;
      }
    }
  }

  ReleaseLayoutDisplayIcon();
  HICON icon = nullptr;
  if (index >= 0) {
    if (auto* layout = bijoy::core::GetLayoutByIndex(index)) {
    icon = LoadLayoutIcon(layout);
      bijoy::core::AddWindowLayoutBinding(GetForegroundWindow(), layout);
    }
  } else {
    bijoy::core::RemoveWindowLayoutBinding(GetForegroundWindow());
  }

  if (!icon) {
    icon = g_defaultIcon;
  }

  g_layoutDisplayIcon = icon;

  if (g_layoutIcon) {
    SendMessageW(g_layoutIcon, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(icon));
  }

  g_notifyIcon.hIcon = icon;
  Shell_NotifyIconW(NIM_MODIFY, &g_notifyIcon);

  if (g_trayMenu) {
    CheckMenuRadioItem(
        g_trayMenu,
        IDM_TRAY_LAYOUT_BASE,
        IDM_TRAY_LAYOUT_BASE + bijoy::core::GetLayoutCount(),
        IDM_TRAY_LAYOUT_BASE + (index + 1),
        MF_BYCOMMAND);
  }
}

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

void PopulateLayoutCombo() {
  if (!g_layoutCombo) {
    return;
  }

  SendMessageW(g_layoutCombo, CB_RESETCONTENT, 0, 0);

  const LRESULT englishIndex = SendMessageW(g_layoutCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
  if (englishIndex >= 0) {
    SendMessageW(g_layoutCombo, CB_SETITEMDATA, static_cast<WPARAM>(englishIndex), static_cast<LPARAM>(-1));
  }

  for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
    if (const auto* layout = bijoy::core::GetLayoutByIndex(i)) {
      const LRESULT comboIndex = SendMessageW(g_layoutCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(layout->name.c_str()));
      if (comboIndex >= 0) {
        SendMessageW(g_layoutCombo, CB_SETITEMDATA, static_cast<WPARAM>(comboIndex), static_cast<LPARAM>(i));
      }
    }
  }

  int currentIndex = bijoy::core::GetCurrentLayoutIndex();
  if (currentIndex >= bijoy::core::GetLayoutCount()) {
    currentIndex = -1;
  }
  SelectLayout(currentIndex);
}

void CycleToNextLayout() {
  const int count = bijoy::core::GetLayoutCount();
  const int current = bijoy::core::GetCurrentLayoutIndex();
  // Cycle: English (-1) -> Layout 0 -> Layout 1 ... -> English (-1)
  int next = current + 1;
  if (next >= count) {
    next = -1;
  }
  SelectLayout(next);
}

void BuildTrayMenu() {
  if (g_trayMenu) {
    DestroyMenu(g_trayMenu);
  }

  g_trayMenu = CreatePopupMenu();

  const bool isVisible = g_mainWindow && IsWindowVisible(g_mainWindow);
  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_SHOW, isVisible ? L"&Hide" : L"&Show");
  AppendMenuW(g_trayMenu, MF_SEPARATOR, 0, nullptr);

  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_LAYOUT_BASE, L"English");

  for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
    if (const auto* layout = bijoy::core::GetLayoutByIndex(i)) {
      AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_LAYOUT_BASE + i + 1, layout->name.c_str());
    }
  }

  AppendMenuW(g_trayMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_EXIT, L"E&xit");
}

void OnTrayCommand(UINT id) {
  if (id == IDM_TRAY_EXIT) {
    if (g_mainWindow) {
      DestroyWindow(g_mainWindow);
    }
    return;
  }

  if (id == IDM_TRAY_SHOW) {
    const bool isVisible = g_mainWindow && IsWindowVisible(g_mainWindow);
    SetMainWindowVisible(!isVisible);
    return;
  }

  if (id >= IDM_TRAY_LAYOUT_BASE && id < IDM_TRAY_LAYOUT_BASE + 101) {
    SelectLayout(static_cast<int>(id - IDM_TRAY_LAYOUT_BASE - 1));
  }
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
        SetMainWindowVisible(false);
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
        SetMainWindowVisible(true);
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
      SetMainWindowVisible(false);
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

  g_mainWindow = CreateWindowExW(
      WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
      windowClass.lpszClassName,
      L"Omor Ekushe",
      WS_POPUPWINDOW,
      CW_USEDEFAULT,
      g_requestedWindowTop,
      440,
      40,
      nullptr,
      nullptr,
      hInstance,
      nullptr);

  return g_mainWindow;
}

void SetMainWindowInitialPosition(int left, int top) {
  g_requestedWindowTop = (top <= 1) ? top : kForcedTopY;

  if (g_mainWindow) {
    const int targetLeft = left >= 0 ? left : 0;
    SetWindowPos(
        g_mainWindow,
        nullptr,
        targetLeft,
        g_requestedWindowTop,
        0,
        0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
  }
}

} // namespace bijoy::platform::windows
