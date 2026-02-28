#include "platform/windows/main_window/main_window_tray.h"
#include "platform/windows/main_window/main_window_types.h"
#include "platform/windows/main_window/main_window_layout.h"
#include "core/app_state.h"

namespace bijoy::platform::windows {

NOTIFYICONDATAW g_notifyIcon = {};
HMENU g_trayMenu = nullptr;
extern HWND g_mainWindow; // Will be defined in main_window.cpp

void BuildTrayMenu() {
  if (g_trayMenu) {
    DestroyMenu(g_trayMenu);
  }

  g_trayMenu = CreatePopupMenu();

  const bool isVisible = g_mainWindow && IsWindowVisible(g_mainWindow);
  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_SHOW, isVisible ? L"&Hide" : L"&Show");
  AppendMenuW(g_trayMenu, MF_SEPARATOR, 0, nullptr);

  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_LAYOUT_BASE, L"English");

  for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
    if (const auto* layout = bijoy::core::GetLayoutByIndex(i)) {
      AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_LAYOUT_BASE + i + 1, layout->name.c_str());
    }
  }

  AppendMenuW(g_trayMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(g_trayMenu, MF_STRING, IDM_TRAY_EXIT, L"E&xit");
}

void OnTrayCommand(UINT id) {
  if (id == IDM_TRAY_EXIT) {
    if (g_mainWindow) {
      DestroyWindow(g_mainWindow);
    }
    return;
  }

  if (id == IDM_TRAY_SHOW) {
    const bool isVisible = g_mainWindow && IsWindowVisible(g_mainWindow);
    ShowWindow(g_mainWindow, isVisible ? SW_HIDE : SW_SHOW);
    return;
  }

  if (id >= IDM_TRAY_LAYOUT_BASE && id <= IDM_TRAY_LAYOUT_BASE + bijoy::core::GetLayoutCount()) {
      SelectLayout(static_cast<int>(id - IDM_TRAY_LAYOUT_BASE - 1));
  }
}

} // namespace bijoy::platform::windows
