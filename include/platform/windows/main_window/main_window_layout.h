#pragma once

#include <windows.h>

namespace bijoy::platform::windows {

extern HWND g_layoutCombo;
extern HWND g_layoutIcon;
extern HICON g_defaultIcon;
extern HICON g_layoutDisplayIcon;

void ReleaseLayoutDisplayIcon();
void SelectLayout(int index);
void PopulateLayoutCombo();
void CycleToNextLayout();

} // namespace bijoy::platform::windows
