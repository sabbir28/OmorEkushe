#include "platform/windows/main_window/main_window_background.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_types.h"
#include "core/layout_discovery.h"
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#include <vector>
#include <algorithm>

namespace bijoy::platform::windows {

HBITMAP g_backgroundBitmap = nullptr;
int g_backgroundOpacity = 72;

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

} // namespace bijoy::platform::windows
