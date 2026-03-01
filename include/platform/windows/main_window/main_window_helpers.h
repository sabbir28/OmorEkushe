#pragma once

#include <string>
#include <windows.h>

namespace bijoy::core { struct Layout; }

namespace bijoy::platform::windows {

std::wstring BuildPath(const std::wstring& base, const wchar_t* relative);
std::wstring BuildPath(const std::wstring& base, const std::wstring& relative);
HICON LoadIconFromPath(const std::wstring& path, int width, int height);
HICON LoadAppIconFromData(int width, int height);
HICON LoadLayoutIcon(const bijoy::core::Layout* layout);
HBITMAP LoadPngAsBitmap(const std::wstring& path, int width, int height);
std::string WideToUtf8(const std::wstring& value);
void SnapWindowToTop(HWND hwnd);

} // namespace bijoy::platform::windows
