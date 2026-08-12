// =============================================================================
// File   : options_overlay.cpp
// Module : platform/windows (Settings Popup Menu)
// Purpose: Implements the settings/options popup menu that appears when the
//          user clicks the gear icon on the main toolbar. Uses native Win32
//          TrackPopupMenu with TPM_RETURNCMD to avoid nested modal conflicts.
//
//          Menu items:
//            - About Omor Ekushe (IDM_ABOUT)
//            - License Information (IDM_LICENSE)
//            - Visit Website (IDM_WEBSITE)
//            - Check for Updates (IDM_UPDATE)
//
//          The selected command is dispatched asynchronously via PostMessageW
//          to the parent window's WM_COMMAND handler, preventing focus loss
//          and window deactivation issues that occurred with synchronous
//          SendMessage during modal dialog ownership chains.
// =============================================================================

#include "platform/windows/options_overlay.h"
#include "platform/windows/resource.h"
#include <windows.h>

namespace bijoy::platform::windows {

void ShowOptionsOverlay(HWND parent, int x, int y) {
    if (!parent || !IsWindow(parent)) {
        return;
    }

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) {
        return;
    }

    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"About Omor Ekushe");
    AppendMenuW(hMenu, MF_STRING, IDM_LICENSE, L"License Information");
    AppendMenuW(hMenu, MF_STRING, IDM_WEBSITE, L"Visit Website");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_UPDATE, L"Check for Updates");

    SetForegroundWindow(parent);
    const UINT selectedCmd = TrackPopupMenu(
        hMenu,
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
        x,
        y,
        0,
        parent,
        nullptr);

    DestroyMenu(hMenu);

    if (selectedCmd > 0) {
        PostMessageW(parent, WM_COMMAND, MAKEWPARAM(selectedCmd, 0), 0);
    }
}

} // namespace bijoy::platform::windows
