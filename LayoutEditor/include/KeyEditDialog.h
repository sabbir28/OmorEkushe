#pragma once
#include <windows.h>
#include "LayoutData.h"

namespace editor {

class KeyEditDialog {
public:
    static bool Show(HWND parent, HINSTANCE hInstance, int keyCode, LayoutData* layout);

private:
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    struct DialogData {
        int keyCode;
        LayoutData* layout;
    };
};

} // namespace editor
