#pragma once

#include <windows.h>

namespace bijoy::core {

bool InstallKeyboardHook(HINSTANCE hInstance);
void UninstallKeyboardHook();
void SetLayoutsReady(bool ready);

} // namespace bijoy::core
