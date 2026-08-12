#pragma once
#include "LayoutData.h"

namespace editor {

class KeyEditDialog {
public:
    static bool Render(LayoutData& layout, int keyCode, bool& show);
};

} // namespace editor
