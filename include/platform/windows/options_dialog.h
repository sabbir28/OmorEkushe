#pragma once

#include <windows.h>

namespace bijoy::platform::windows {

/// Displays the modal/owned Settings & Options configuration dialog.
/// If already open, brings the existing dialog window to the foreground.
void ShowOptionsDialog(HWND parent);

} // namespace bijoy::platform::windows

