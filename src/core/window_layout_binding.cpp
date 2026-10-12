// =============================================================================
// File   : window_layout_binding.cpp
// Module : core (Per-Window Layout Binding)
// Purpose: Manages a runtime mapping between individual NativeWindowHandle
//          windows and their assigned keyboard layouts across Windows,
//          Linux, and macOS.
// =============================================================================

#include "core/window_layout_binding.h"

#include <vector>

namespace bijoy::core {

    namespace {

        std::vector<WindowLayoutBinding> g_bindings;

        bool IsWindowAlive(NativeWindowHandle handle) {
#if defined(_WIN32)
            return IsWindow(handle) != FALSE;
#else
            return handle != kInvalidWindowHandle;
#endif
        }

    } // namespace

    bool AddWindowLayoutBinding(NativeWindowHandle hwnd, Layout* layout) {
        for (auto it = g_bindings.begin(); it != g_bindings.end();) {
            if (it->handle == hwnd) {
                it = g_bindings.erase(it);
            } else {
                ++it;
            }
        }

        g_bindings.push_back({hwnd, layout});

        for (auto it = g_bindings.begin(); it != g_bindings.end();) {
            if (!IsWindowAlive(it->handle)) {
                it = g_bindings.erase(it);
            } else {
                ++it;
            }
        }

        return true;
    }

    Layout* FindWindowLayoutBinding(NativeWindowHandle hwnd) {
        for (auto& entry : g_bindings) {
            if (entry.handle == hwnd) {
                return entry.status;
            }
        }
        return nullptr;
    }

    bool RemoveWindowLayoutBinding(NativeWindowHandle hwnd) {
        for (auto it = g_bindings.begin(); it != g_bindings.end(); ++it) {
            if (it->handle == hwnd) {
                g_bindings.erase(it);
                return true;
            }
        }
        return false;
    }

} // namespace bijoy::core

