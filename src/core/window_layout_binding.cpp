#include "core/window_layout_binding.h"

#include <vector>

namespace bijoy::core {

namespace {

std::vector<WindowLayoutBinding> g_bindings;

} // namespace

bool AddWindowLayoutBinding(HWND hwnd, Layout* layout) {
  for (auto it = g_bindings.begin(); it != g_bindings.end();) {
    if (it->handle == hwnd) {
      it = g_bindings.erase(it);
    } else {
      ++it;
    }
  }

  g_bindings.push_back({hwnd, layout});

  for (auto it = g_bindings.begin(); it != g_bindings.end();) {
    if (!IsWindow(it->handle)) {
      it = g_bindings.erase(it);
    } else {
      ++it;
    }
  }

  return true;
}

Layout* FindWindowLayoutBinding(HWND hwnd) {
  for (auto& entry : g_bindings) {
    if (entry.handle == hwnd) {
      return entry.status;
    }
  }
  return nullptr;
}

bool RemoveWindowLayoutBinding(HWND hwnd) {
  for (auto it = g_bindings.begin(); it != g_bindings.end(); ++it) {
    if (it->handle == hwnd) {
      g_bindings.erase(it);
      return true;
    }
  }
  return false;
}

} // namespace bijoy::core
