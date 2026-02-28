#pragma once
#include <windows.h>

class MainWindow {
public:
    static bool Register(HINSTANCE hInstance);
    static HWND Create(HINSTANCE hInstance, int nCmdShow);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void CreateControls(HWND hwnd, HINSTANCE hInstance);
    static void ApplySystemFont(HWND hwnd);
    static void OnDestroy(HWND hwnd);
};
