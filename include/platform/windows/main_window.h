#pragma once

#include <windows.h>

namespace bijoy::platform::windows {

HWND CreateMainWindow(HINSTANCE hInstance);
void SetMainWindowInitialPosition(int left, int top);

} // namespace bijoy::platform::windows
