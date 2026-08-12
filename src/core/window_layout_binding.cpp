// =============================================================================
// File   : window_layout_binding.cpp
// Module : core (Per-Window Layout Binding)
// Purpose: Manages a runtime mapping between individual HWND windows and their
//          assigned keyboard layouts. This enables per-application layout
//          memory: when the user activates a layout while typing in Notepad,
//          switching to Chrome and back preserves the Notepad layout.
//
//          The binding list is self-cleaning: stale entries for destroyed
//          windows (identified by IsWindow() returning FALSE) are automatically
//          pruned on each Add operation.
//
//          Data structure: Simple linear vector scan. Adequate for the typical
//          small number of simultaneously bound windows (<20).
// =============================================================================

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
