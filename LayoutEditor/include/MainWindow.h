#pragma once
#include <vector>
#include "LayoutData.h"

class MainWindow {
public:
    static void Initialize();
    static bool Render();
    static void Shutdown();

private:
    static void RefreshLayouts();
    static void UpdateLayoutView();
    
    static std::vector<editor::LayoutData> s_layouts;
    static int s_currentLayoutIndex;
    static char s_editName[256];
    static char s_editIcon[256];
    static bool s_showKeyEditPopup;
    static int s_selectedKeyCode;
};
