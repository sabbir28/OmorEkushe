#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
using HINSTANCE = void*;
#endif

namespace bijoy::core {

	bool InstallKeyboardHook(HINSTANCE hInstance);
	void UninstallKeyboardHook();
	void SetLayoutsReady(bool ready);

	void SetPhoneticToggleShortcut(int vkCode, bool ctrl, bool alt, bool shift);
	void GetPhoneticToggleShortcut(int& vkCode, bool& ctrl, bool& alt, bool& shift);

} // namespace bijoy::core

