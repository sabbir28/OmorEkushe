#pragma once

#include "core/layout.h"
#include <vector>

namespace bijoy::core {

extern std::vector<Layout> g_layouts;
extern int g_layoutCount;
extern int g_currentLayoutIndex;
extern int g_comLayoutSelectedIndex;

void SetCurrentLayout(int index);
int GetCurrentLayoutIndex();
Layout* GetCurrentLayout();
Layout* GetLayoutByIndex(int index);
int GetLayoutCount();

} // namespace bijoy::core
