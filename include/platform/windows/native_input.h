#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace bijoy::platform::windows {

void DoKeyboard(DWORD flags, int scanCode);
void DoKeyboardVk(DWORD flags, WORD vk);

} // namespace bijoy::platform::windows
