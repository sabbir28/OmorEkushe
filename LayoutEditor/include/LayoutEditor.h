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
constexpr int APP_HEIGHT = 450;

// Control IDs
constexpr int IDC_COMBO_LAYOUT = 1001;
constexpr int IDC_EDIT_NAME    = 1002;
constexpr int IDC_BTN_NEW      = 1010;
constexpr int IDC_BTN_SAVE     = 1011;
constexpr int IDC_BTN_CANCEL   = 1012;