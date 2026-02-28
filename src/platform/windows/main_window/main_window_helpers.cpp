#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_types.h"
#include "core/layout.h"
#include "core/layout_discovery.h"

#include <algorithm>

namespace bijoy::platform::windows {

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

} // namespace bijoy::platform::windows
