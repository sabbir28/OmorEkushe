#pragma once
#include "imgui.h"
#include "LayoutData.h"

namespace editor {

extern ImFont* g_FontUI;
extern ImFont* g_FontKeyLarge;
extern ImFont* g_FontKeySub;
extern ImFont* g_FontKeySmall;
extern bool g_DarkMode;

class KeyboardView {
public:
    static bool Render(const LayoutData& layout, ImVec2 size, int& clickedKeyCode);
};

} // namespace editor

