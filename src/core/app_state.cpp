// =============================================================================
// File   : app_state.cpp
// Module : core (Global Application State)
// Purpose: Owns the global mutable state for Omor Ekushe's active keyboard
//          layouts. Provides centralized accessors and mutators for:
//            - g_layouts: vector of all loaded Layout definitions
//            - g_currentLayoutIndex: index of the currently active layout (-1 = English)
//            - g_comLayoutSelectedIndex: 1-based index used by COM/tray UI (0 = English)
//
//          Thread Safety: NOT thread-safe. All access occurs on the main UI thread.
// =============================================================================

#include "core/app_state.h"

namespace bijoy::core {

    // ---- Global layout state ------------------------------------------------
    std::vector<Layout> g_layouts;                 // All loaded keyboard layouts
    int g_layoutCount = 0;                         // (Legacy) Cached layout count
    int g_currentLayoutIndex = -1;                 // Active layout index (-1 = English/pass-through)
    int g_comLayoutSelectedIndex = 0;              // 1-based selection index for tray/COM UI

    /// Sets the active keyboard layout by zero-based index.
    /// Pass -1 to deactivate all layouts (revert to English pass-through).
    void SetCurrentLayout(int index) {
        g_currentLayoutIndex = index;
        g_comLayoutSelectedIndex = index < 0 ? 0 : index + 1;
    }

    int GetCurrentLayoutIndex() {
        return g_currentLayoutIndex;
    }

    Layout* GetCurrentLayout() {
        if (g_currentLayoutIndex < 0 || g_currentLayoutIndex >= static_cast<int>(g_layouts.size())) {
            return nullptr;
        }
        return &g_layouts[g_currentLayoutIndex];
    }

    Layout* GetLayoutByIndex(int index) {
        if (index < 0 || index >= static_cast<int>(g_layouts.size())) {
            return nullptr;
        }
        return &g_layouts[index];
    }

    int GetLayoutCount() {
        return static_cast<int>(g_layouts.size());
    }

} // namespace bijoy::core
