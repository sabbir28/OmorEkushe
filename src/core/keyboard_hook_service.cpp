#include "core/keyboard_hook_service.h"

#include "core/app_state.h"
#include "core/window_layout_binding.h"
#include "platform/windows/native_input.h"

#include <string>
#include <windows.h>

namespace bijoy::core {

namespace {

struct KBDLLHOOKSTRUCT_LOCAL {
  DWORD vkCode;
  DWORD scanCode;
  DWORD flags;
  DWORD time;
  ULONG_PTR dwExtraInfo;
};

HHOOK g_hook = nullptr;
bool g_layoutsReady = false;

bool IsKeyPressed(int vk) {
  return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

bool ProcessHookedEvent(const KBDLLHOOKSTRUCT_LOCAL* hs) {
  const bool ctrl = IsKeyPressed(VK_CONTROL);
  const bool alt = IsKeyPressed(VK_MENU);
  const bool shift = IsKeyPressed(VK_SHIFT);

  for (int i = 0; i < GetLayoutCount(); ++i) {
    Layout* layout = GetLayoutByIndex(i);
    if (!layout) {
      continue;
    }

    if (ctrl == layout->shortcut.ctrl && alt == layout->shortcut.alt &&
        shift == layout->shortcut.shift && static_cast<DWORD>(layout->shortcut.keyCode) == hs->vkCode) {
      const HWND foregroundWindow = GetForegroundWindow();
      if (g_comLayoutSelectedIndex == i + 1) {
        RemoveWindowLayoutBinding(foregroundWindow);
        SetCurrentLayout(-1);
      } else {
        SetCurrentLayout(i);
        AddWindowLayoutBinding(foregroundWindow, layout);
      }
      return true;
    }
  }

  Layout* activeLayout = GetCurrentLayout();
  if (!ctrl && !alt && activeLayout) {
    auto it = activeLayout->key.find(static_cast<int>(hs->vkCode));
    if (it != activeLayout->key.end()) {
      const std::wstring& output = shift ? it->second.shift : it->second.normal;
      if (!output.empty()) {
        for (wchar_t c : output) {
          bijoy::platform::windows::DoKeyboard(
              KEYEVENTF_UNICODE,
              static_cast<int>(c));
          bijoy::platform::windows::DoKeyboard(
              KEYEVENTF_KEYUP | KEYEVENTF_UNICODE,
              static_cast<int>(c));
        }
        return true;
      }
    }
  }

  return false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && g_layoutsReady && lParam != 0) {
    auto* hs = reinterpret_cast<KBDLLHOOKSTRUCT_LOCAL*>(lParam);
    if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && ProcessHookedEvent(hs)) {
      return 1;
    }
  }
  return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

} // namespace

bool InstallKeyboardHook(HINSTANCE hInstance) {
  g_hook = SetWindowsHookExW(13, LowLevelKeyboardProc, hInstance, 0);
  return g_hook != nullptr;
}

void UninstallKeyboardHook() {
  if (g_hook) {
    UnhookWindowsHookEx(g_hook);
    g_hook = nullptr;
  }
}

void SetLayoutsReady(bool ready) {
  g_layoutsReady = ready;
}

} // namespace bijoy::core
