#pragma once

#include <windows.h>

namespace bijoy::core {

	bool InstallKeyboardHook(HINSTANCE hInstance);
	void UninstallKeyboardHook();
	void SetLayoutsReady(bool ready);

	void SetPhoneticToggleShortcut(int vkCode, bool ctrl, bool alt, bool shift);
	void GetPhoneticToggleShortcut(int& vkCode, bool& ctrl, bool& alt, bool& shift);

} // namespace bijoy::core
