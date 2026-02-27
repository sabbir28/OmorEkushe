#include "platform/windows/native_input.h"

namespace bijoy::platform::windows {

void DoKeyboard(DWORD flags, int scanCode) {
  INPUT input = {};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = 0;
  input.ki.wScan = static_cast<WORD>(scanCode);
  input.ki.dwFlags = flags;
  ::SendInput(1, &input, sizeof(INPUT));
}

void DoKeyboardVk(DWORD flags, WORD vk) {
  INPUT input = {};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = vk;
  input.ki.wScan = 0;
  input.ki.dwFlags = flags;
  ::SendInput(1, &input, sizeof(INPUT));
}

} // namespace bijoy::platform::windows
