#pragma once

#include "core/platform_types.h"
#include <string>

namespace bijoy::core {

/// Cross-platform input injection interface
void PlatformEmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend);

/// Returns the handle of the currently focused/active foreground window
NativeWindowHandle PlatformGetActiveWindow();

/// Simulates a virtual/raw key press or release
void PlatformSimulateKey(int keyCode, bool isKeyDown);

} // namespace bijoy::core

