#pragma once

#include <windows.h>

namespace bijoy::platform::windows {

	HWND CreateMainWindow(HINSTANCE hInstance);
	void SetMainWindowInitialPosition(int left, int top);
	void SetMainWindowOpacity(int opacityPercent);
	int GetMainWindowOpacity();
	void SetMainWindowAlwaysOnTop(bool alwaysOnTop);
	void SetMainWindowSnapToTop(bool snap);
	void ResetMainWindowPosition();

} // namespace bijoy::platform::windows
