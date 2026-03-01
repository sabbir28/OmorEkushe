#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_types.h"
#include "core/layout.h"
#include "core/layout_discovery.h"
#include "lib/stb_image.h"

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

HBITMAP LoadPngAsBitmap(const std::wstring& path, int width, int height) {
  const std::string pathUtf8 = WideToUtf8(path);
  if (pathUtf8.empty()) {
    return nullptr;
  }

  int srcWidth = 0, srcHeight = 0, srcChannels = 0;
  stbi_uc* data = stbi_load(pathUtf8.c_str(), &srcWidth, &srcHeight, &srcChannels, 4);
  if (!data) {
    return nullptr;
  }

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height; // Top-down
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = nullptr;
  HDC screenDc = GetDC(nullptr);
  HBITMAP hBitmap = CreateDIBSection(screenDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
  
  if (hBitmap && bits) {
    HDC srcDc = CreateCompatibleDC(screenDc);
    HDC dstDc = CreateCompatibleDC(screenDc);

    BITMAPINFO srcBmi = {};
    srcBmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    srcBmi.bmiHeader.biWidth = srcWidth;
    srcBmi.bmiHeader.biHeight = -srcHeight;
    srcBmi.bmiHeader.biPlanes = 1;
    srcBmi.bmiHeader.biBitCount = 32;
    srcBmi.bmiHeader.biCompression = BI_RGB;

    void* srcBits = nullptr;
    HBITMAP hSrcBitmap = CreateDIBSection(screenDc, &srcBmi, DIB_RGB_COLORS, &srcBits, nullptr, 0);
    
    if (hSrcBitmap && srcBits) {
      // Convert RGBA to BGRA and Pre-multiply Alpha for the source
      auto* srcPixels = static_cast<unsigned char*>(srcBits);
      for (int i = 0; i < srcWidth * srcHeight; ++i) {
        unsigned char r = data[i * 4 + 0];
        unsigned char g = data[i * 4 + 1];
        unsigned char b = data[i * 4 + 2];
        unsigned char a = data[i * 4 + 3];

        // BGRA order for DIB section and pre-multiplying alpha
        srcPixels[i * 4 + 0] = static_cast<unsigned char>((b * a) / 255);
        srcPixels[i * 4 + 1] = static_cast<unsigned char>((g * a) / 255);
        srcPixels[i * 4 + 2] = static_cast<unsigned char>((r * a) / 255);
        srcPixels[i * 4 + 3] = a;
      }

      const HGDIOBJ oldSrc = SelectObject(srcDc, hSrcBitmap);
      const HGDIOBJ oldDst = SelectObject(dstDc, hBitmap);
      
      // Use AlphaBlend for resizing to properly handle pre-multiplied alpha channel
      BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
      AlphaBlend(dstDc, 0, 0, width, height, srcDc, 0, 0, srcWidth, srcHeight, bf);

      SelectObject(srcDc, oldSrc);
      SelectObject(dstDc, oldDst);
      DeleteObject(hSrcBitmap);
    }
    
    DeleteDC(srcDc);
    DeleteDC(dstDc);
  }

  ReleaseDC(nullptr, screenDc);
  stbi_image_free(data);
  return hBitmap;
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
