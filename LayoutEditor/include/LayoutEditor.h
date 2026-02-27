#pragma once
#include <windows.h>

struct KeyDef {
    const wchar_t* normal;
    const wchar_t* shifted;
    int   x;
    int   y;
    float width;
    float height;
};

constexpr int APP_WIDTH  = 600;
constexpr int APP_HEIGHT = 400;

// Control IDs
constexpr int IDC_COMBO_LAYOUT = 1001;
constexpr int IDC_EDIT_NAME    = 1002;
constexpr int IDC_BTN_NEW      = 1010;
constexpr int IDC_BTN_SAVE     = 1011;
constexpr int IDC_BTN_CANCEL   = 1012;

// Globals (explicit ownership)
extern HINSTANCE g_hInstance;

// Prototypes
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateMainControls(HWND hwnd);