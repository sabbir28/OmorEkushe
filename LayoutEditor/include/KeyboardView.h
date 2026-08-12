#pragma once
#include "imgui.h"
#include "LayoutData.h"

namespace editor {

class KeyboardView {
public:
    static bool Render(const LayoutData& layout, ImVec2 size, int& clickedKeyCode);
};

} // namespace editor
