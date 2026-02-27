#include "core/app_state.h"

namespace bijoy::core {

std::vector<Layout> g_layouts;
int g_layoutCount = 0;
int g_currentLayoutIndex = -1;
int g_comLayoutSelectedIndex = 0;

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
