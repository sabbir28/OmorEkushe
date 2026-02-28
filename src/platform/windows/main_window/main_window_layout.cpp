#include "platform/windows/main_window/main_window_layout.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/main_window/main_window_types.h"
#include "core/app_state.h"
#include "core/window_layout_binding.h"

#include <shellapi.h>

namespace bijoy::platform::windows {

HWND g_layoutCombo = nullptr;
HWND g_layoutIcon = nullptr;
HICON g_defaultIcon = nullptr;
HICON g_layoutDisplayIcon = nullptr;
extern NOTIFYICONDATAW g_notifyIcon; // Will be defined in tray module
extern HMENU g_trayMenu; // Will be defined in tray module

void ReleaseLayoutDisplayIcon() {
  if (g_layoutDisplayIcon && g_layoutDisplayIcon != g_defaultIcon) {
    DestroyIcon(g_layoutDisplayIcon);
  }
  g_layoutDisplayIcon = nullptr;
}

void SelectLayout(int index) {
  bijoy::core::SetCurrentLayout(index);

  if (g_layoutCombo) {
    const int count = static_cast<int>(SendMessageW(g_layoutCombo, CB_GETCOUNT, 0, 0));
    for (int comboIndex = 0; comboIndex < count; ++comboIndex) {
      const LRESULT itemData = SendMessageW(g_layoutCombo, CB_GETITEMDATA, static_cast<WPARAM>(comboIndex), 0);
      if (static_cast<int>(itemData) == index) {
        SendMessageW(g_layoutCombo, CB_SETCURSEL, static_cast<WPARAM>(comboIndex), 0);
        break;
      }
    }
  }

  ReleaseLayoutDisplayIcon();
  HICON icon = nullptr;
  if (index >= 0) {
    if (auto* layout = bijoy::core::GetLayoutByIndex(index)) {
      icon = LoadLayoutIcon(layout);
      bijoy::core::AddWindowLayoutBinding(GetForegroundWindow(), layout);
    }
  } else {
    bijoy::core::RemoveWindowLayoutBinding(GetForegroundWindow());
  }

  if (!icon) {
    icon = g_defaultIcon;
  }

  g_layoutDisplayIcon = icon;

  if (g_layoutIcon) {
    SendMessageW(g_layoutIcon, STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(icon));
  }

  g_notifyIcon.hIcon = icon;
  Shell_NotifyIconW(NIM_MODIFY, &g_notifyIcon);

  if (g_trayMenu) {
    CheckMenuRadioItem(
        g_trayMenu,
        IDM_TRAY_LAYOUT_BASE,
        IDM_TRAY_LAYOUT_BASE + bijoy::core::GetLayoutCount(),
        IDM_TRAY_LAYOUT_BASE + (index + 1),
        MF_BYCOMMAND);
  }
}

void PopulateLayoutCombo() {
  if (!g_layoutCombo) {
    return;
  }

  SendMessageW(g_layoutCombo, CB_RESETCONTENT, 0, 0);

  const LRESULT englishIndex = SendMessageW(g_layoutCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
  if (englishIndex >= 0) {
    SendMessageW(g_layoutCombo, CB_SETITEMDATA, static_cast<WPARAM>(englishIndex), static_cast<LPARAM>(-1));
  }

  for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
    if (const auto* layout = bijoy::core::GetLayoutByIndex(i)) {
      const LRESULT comboIndex = SendMessageW(g_layoutCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(layout->name.c_str()));
      if (comboIndex >= 0) {
        SendMessageW(g_layoutCombo, CB_SETITEMDATA, static_cast<WPARAM>(comboIndex), static_cast<LPARAM>(i));
      }
    }
  }

  int currentIndex = bijoy::core::GetCurrentLayoutIndex();
  if (currentIndex >= bijoy::core::GetLayoutCount()) {
    currentIndex = -1;
  }
  SelectLayout(currentIndex);
}

void CycleToNextLayout() {
  const int count = bijoy::core::GetLayoutCount();
  const int current = bijoy::core::GetCurrentLayoutIndex();
  // Cycle: English (-1) -> Layout 0 -> Layout 1 ... -> English (-1)
  int next = current + 1;
  if (next >= count) {
    next = -1;
  }
  SelectLayout(next);
}

} // namespace bijoy::platform::windows
