#pragma once
#include <windows.h>
#include <vector>
#include "LayoutData.h"

class MainWindow {
public:
    static bool Register(HINSTANCE hInstance);
    static HWND Create(HINSTANCE hInstance, int nCmdShow);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void CreateControls(HWND hwnd, HINSTANCE hInstance);
    static void ApplySystemFont(HWND hwnd);
    static void OnDestroy(HWND hwnd);
    static void RefreshComboBox(HWND hwnd);
    static void UpdateLayoutView(HWND hwnd);
    
    static std::vector<editor::LayoutData> s_layouts;
    static int s_currentLayoutIndex;
    static HWND s_hCombo;
    static HWND s_hKeyboardView;
    static HWND s_hEditName;
};
