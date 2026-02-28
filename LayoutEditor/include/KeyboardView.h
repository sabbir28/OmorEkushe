#pragma once

#include <windows.h>
#include "LayoutData.h"

namespace editor {

class KeyboardView {
public:
    static bool Register(HINSTANCE hInstance);
    static HWND Create(HWND parent, HINSTANCE hInstance, int x, int y, int width, int height, int id);

    static void SetLayout(HWND hwnd, const LayoutData* layout);
    static const LayoutData* GetLayout(HWND hwnd);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static void OnPaint(HWND hwnd);
    static void OnLButtonDown(HWND hwnd, int x, int y);
};

} // namespace editor
