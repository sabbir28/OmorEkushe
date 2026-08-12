#pragma once

#include <windows.h>
#include <string>

namespace bijoy::platform::windows {

extern HBITMAP g_backgroundBitmap;
extern int g_backgroundOpacity;

void ReleaseBackgroundBitmap();
std::wstring ResolveBackgroundImagePath();
void LoadBackgroundBitmapForWindow(HWND hwnd);
void DrawWindowBackground(HWND hwnd, HDC targetDc);

} // namespace bijoy::platform::windows
