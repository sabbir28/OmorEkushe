#pragma once

#include <windows.h>
#include <shellapi.h>

namespace bijoy::platform::windows {

extern NOTIFYICONDATAW g_notifyIcon;
extern HMENU g_trayMenu;

void BuildTrayMenu();
void OnTrayCommand(UINT id);

} // namespace bijoy::platform::windows
