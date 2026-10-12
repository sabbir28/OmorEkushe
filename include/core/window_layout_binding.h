#pragma once

#include "core/layout.h"
#include "core/platform_types.h"

namespace bijoy::core {

struct WindowLayoutBinding {
  NativeWindowHandle handle = kInvalidWindowHandle;
  Layout* status = nullptr;
};

bool AddWindowLayoutBinding(NativeWindowHandle hwnd, Layout* layout);
Layout* FindWindowLayoutBinding(NativeWindowHandle hwnd);
bool RemoveWindowLayoutBinding(NativeWindowHandle hwnd);

} // namespace bijoy::core

