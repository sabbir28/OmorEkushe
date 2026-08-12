#pragma once

#include "core/layout.h"
#include <windows.h>

namespace bijoy::core {

struct WindowLayoutBinding {
  HWND handle = nullptr;
  Layout* status = nullptr;
};

bool AddWindowLayoutBinding(HWND hwnd, Layout* layout);
Layout* FindWindowLayoutBinding(HWND hwnd);
bool RemoveWindowLayoutBinding(HWND hwnd);

} // namespace bijoy::core
