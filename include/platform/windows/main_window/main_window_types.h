#pragma once

#include <windows.h>
#include <commctrl.h>

#ifndef TPM_NONBLOCKING
#define TPM_NONBLOCKING 0x0000L
#endif

namespace bijoy::platform::windows {

// Message IDs
constexpr UINT kTrayIconMessage = WM_USER + 1;

// Control IDs
constexpr UINT_PTR IDC_LAYOUT_ICON = 101;
constexpr UINT_PTR IDC_LAYOUT_COMBO = 102;
constexpr UINT_PTR IDC_OPTIONS_BUTTON = 103;
constexpr UINT_PTR IDC_LAYOUT_EDITOR_BUTTON = 104;
constexpr UINT_PTR IDC_MINIMIZE_BUTTON = 105;
constexpr UINT_PTR IDC_CLOSE_BUTTON = 106;

// Menu IDs
constexpr UINT IDM_TRAY_SHOW = 201;
constexpr UINT IDM_TRAY_EXIT = 202;
constexpr UINT IDM_TRAY_LAYOUT_BASE = 300; // Layouts will be BASE + index + 1

// Tray Icon ID
constexpr UINT ID_TRAY = 1;

// Constants
constexpr int kForcedTopY = 0;

} // namespace bijoy::platform::windows
