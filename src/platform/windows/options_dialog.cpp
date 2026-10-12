// =============================================================================
// File   : options_dialog.cpp
// Module : platform/windows (Full Settings & Configuration UI)
// Purpose: Implements the full-featured settings and configuration dialog
//          for Omor Ekushe. Provides tabs for:
//            - General & Startup: Auto-start with Windows, tray minimization,
//              splash screen, default layout on launch, per-window binding.
//            - Hotkeys & Shortcuts: Custom Bangla phonetic toggle hotkey
//              (F12, Space, Caps, etc. + Ctrl/Alt/Shift), and per-layout switch
//              shortcuts (Bijoy Unicode, Bijoy Classic, etc.).
//            - Appearance & Overlay: Real-time floating toolbar opacity /
//              transparency slider (20% - 100%), always-on-top toggle, snap to
//              top edge toggle, and position reset.
//            - About & Updates: Version info, GitHub links, NetClient updater.
//
//          Design: Pure native Win32 controls with Segoe UI font rendering,
//          double-buffered tab switching, and instant live preview.
// =============================================================================

#include "platform/windows/options_dialog.h"
#include "platform/windows/main_window.h"
#include "platform/windows/main_window/main_window_helpers.h"
#include "platform/windows/resource.h"
#include "core/startup_options.h"
#include "core/app_state.h"
#include "core/keyboard_hook_service.h"
#include "core/layout_discovery.h"
#include "utils/system_utils.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <algorithm>

namespace bijoy::platform::windows {

namespace {

constexpr wchar_t kOptionsDialogClass[] = L"OmorEkusheSettingsDialog";
constexpr int kDialogWidth = 530;
constexpr int kDialogHeight = 490;

// Tab control ID
constexpr UINT_PTR IDC_OPT_TAB = 5001;

// General Tab (5100s)
constexpr UINT_PTR IDC_OPT_AUTOSTART = 5101;
constexpr UINT_PTR IDC_OPT_TRAYMODE = 5102;
constexpr UINT_PTR IDC_OPT_SPLASH = 5103;
constexpr UINT_PTR IDC_OPT_DEFAULT_LAYOUT = 5104;
constexpr UINT_PTR IDC_OPT_WINDOW_BINDING = 5105;

// Hotkeys Tab (5200s)
constexpr UINT_PTR IDC_OPT_PHONETIC_KEY = 5201;
constexpr UINT_PTR IDC_OPT_PHONETIC_CTRL = 5202;
constexpr UINT_PTR IDC_OPT_PHONETIC_ALT = 5203;
constexpr UINT_PTR IDC_OPT_PHONETIC_SHIFT = 5204;
constexpr UINT_PTR IDC_OPT_PHONETIC_STATUS = 5205;
constexpr UINT_PTR IDC_OPT_LAYOUT_SEL = 5206;
constexpr UINT_PTR IDC_OPT_LAYOUT_KEY = 5207;
constexpr UINT_PTR IDC_OPT_LAYOUT_CTRL = 5208;
constexpr UINT_PTR IDC_OPT_LAYOUT_ALT = 5209;
constexpr UINT_PTR IDC_OPT_LAYOUT_SHIFT = 5210;
constexpr UINT_PTR IDC_OPT_LAYOUT_ASSIGN = 5211;
constexpr UINT_PTR IDC_OPT_LAYOUT_RESET = 5212;
constexpr UINT_PTR IDC_OPT_LAYOUT_LIST = 5213;

// Appearance Tab (5300s)
constexpr UINT_PTR IDC_OPT_OPACITY_SLIDER = 5301;
constexpr UINT_PTR IDC_OPT_OPACITY_LABEL = 5302;
constexpr UINT_PTR IDC_OPT_OPACITY_50 = 5303;
constexpr UINT_PTR IDC_OPT_OPACITY_75 = 5304;
constexpr UINT_PTR IDC_OPT_OPACITY_100 = 5305;
constexpr UINT_PTR IDC_OPT_TOPMOST = 5306;
constexpr UINT_PTR IDC_OPT_SNAP_TOP = 5307;
constexpr UINT_PTR IDC_OPT_RESET_POS = 5308;

// About Tab (5400s)
constexpr UINT_PTR IDC_OPT_BTN_UPDATE = 5401;
constexpr UINT_PTR IDC_OPT_BTN_GITHUB = 5402;
constexpr UINT_PTR IDC_OPT_BTN_LICENSE = 5403;
constexpr UINT_PTR IDC_OPT_BTN_EDITOR = 5404;

// Dialog Action Buttons (5500s)
constexpr UINT_PTR IDC_OPT_BTN_OK = 5501;
constexpr UINT_PTR IDC_OPT_BTN_CANCEL = 5502;
constexpr UINT_PTR IDC_OPT_BTN_APPLY = 5503;

struct KeyOption {
    const wchar_t* name;
    int vkCode;
};

const KeyOption kPhoneticKeys[] = {
    { L"F12", VK_F12 },
    { L"F11", VK_F11 },
    { L"F10", VK_F10 },
    { L"F9",  VK_F9  },
    { L"F8",  VK_F8  },
    { L"F7",  VK_F7  },
    { L"Space", VK_SPACE },
    { L"` ~ (Grave / Tilde)", VK_OEM_3 },
    { L"Caps Lock", VK_CAPITAL },
    { L"Tab", VK_TAB }
};

const KeyOption kLayoutKeys[] = {
    { L"V", 'V' }, { L"B", 'B' }, { L"P", 'P' }, { L"E", 'E' },
    { L"A", 'A' }, { L"C", 'C' }, { L"D", 'D' }, { L"F", 'F' },
    { L"G", 'G' }, { L"H", 'H' }, { L"I", 'I' }, { L"J", 'J' },
    { L"K", 'K' }, { L"L", 'L' }, { L"M", 'M' }, { L"N", 'N' },
    { L"O", 'O' }, { L"Q", 'Q' }, { L"R", 'R' }, { L"S", 'S' },
    { L"T", 'T' }, { L"U", 'U' }, { L"W", 'W' }, { L"X", 'X' },
    { L"Y", 'Y' }, { L"Z", 'Z' },
    { L"F1", VK_F1 }, { L"F2", VK_F2 }, { L"F3", VK_F3 }, { L"F4", VK_F4 },
    { L"F5", VK_F5 }, { L"F6", VK_F6 }, { L"F7", VK_F7 }, { L"F8", VK_F8 },
    { L"F9", VK_F9 }, { L"F10", VK_F10 }, { L"F11", VK_F11 }, { L"F12", VK_F12 }
};

std::wstring FormatShortcutString(int vk, bool ctrl, bool alt, bool shift) {
    std::wstring str;
    if (ctrl) str += L"Ctrl + ";
    if (alt) str += L"Alt + ";
    if (shift) str += L"Shift + ";

    for (const auto& ko : kLayoutKeys) {
        if (ko.vkCode == vk) {
            str += ko.name;
            return str;
        }
    }
    for (const auto& ko : kPhoneticKeys) {
        if (ko.vkCode == vk) {
            str += ko.name;
            return str;
        }
    }
    wchar_t buf[32] = {};
    swprintf_s(buf, L"Key 0x%X", vk);
    str += buf;
    return str;
}

HWND g_settingsDialog = nullptr;

struct DialogState {
    HWND parent = nullptr;
    HWND hTab = nullptr;

    // Fonts
    HFONT hFontNormal = nullptr;
    HFONT hFontBold = nullptr;
    HFONT hFontTitle = nullptr;

    // Controls per tab
    std::vector<HWND> tabControls[4];

    // General tab controls
    HWND hAutoStart = nullptr;
    HWND hTrayMode = nullptr;
    HWND hSplash = nullptr;
    HWND hDefaultLayout = nullptr;
    HWND hWindowBinding = nullptr;

    // Hotkeys tab controls
    HWND hPhoneticKey = nullptr;
    HWND hPhoneticCtrl = nullptr;
    HWND hPhoneticAlt = nullptr;
    HWND hPhoneticShift = nullptr;
    HWND hPhoneticStatus = nullptr;

    HWND hLayoutSel = nullptr;
    HWND hLayoutKey = nullptr;
    HWND hLayoutCtrl = nullptr;
    HWND hLayoutAlt = nullptr;
    HWND hLayoutShift = nullptr;
    HWND hLayoutSummary = nullptr;

    // Appearance tab controls
    HWND hOpacitySlider = nullptr;
    HWND hOpacityLabel = nullptr;
    HWND hTopmost = nullptr;
    HWND hSnapTop = nullptr;

    // Snapshot of options at dialog open (for Cancel revert)
    bijoy::core::StartupOptions initialOptions;
    bijoy::core::StartupOptions currentOptions;
};

void UpdatePhoneticStatusBadge(DialogState* state) {
    if (!state || !state->hPhoneticStatus) return;
    int curSel = static_cast<int>(SendMessageW(state->hPhoneticKey, CB_GETCURSEL, 0, 0));
    int vk = VK_F12;
    if (curSel >= 0) {
        vk = static_cast<int>(SendMessageW(state->hPhoneticKey, CB_GETITEMDATA, curSel, 0));
    }
    bool ctrl = (Button_GetCheck(state->hPhoneticCtrl) == BST_CHECKED);
    bool alt = (Button_GetCheck(state->hPhoneticAlt) == BST_CHECKED);
    bool shift = (Button_GetCheck(state->hPhoneticShift) == BST_CHECKED);

    std::wstring status = L"Current Quick Toggle:  [ " + FormatShortcutString(vk, ctrl, alt, shift) + L" ]";
    SetWindowTextW(state->hPhoneticStatus, status.c_str());
}

void RefreshLayoutShortcutsSummary(DialogState* state) {
    if (!state || !state->hLayoutSummary) return;

    std::wstring summary = L"Active Layout Shortcuts:\r\n";
    for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
        const auto* l = bijoy::core::GetLayoutByIndex(i);
        if (l) {
            summary += L" • " + l->name + L": " +
                       FormatShortcutString(l->shortcut.keyCode, l->shortcut.ctrl, l->shortcut.alt, l->shortcut.shift) +
                       L"\r\n";
        }
    }
    SetWindowTextW(state->hLayoutSummary, summary.c_str());
}

void SyncSelectedLayoutControls(DialogState* state) {
    if (!state || !state->hLayoutSel) return;

    int sel = static_cast<int>(SendMessageW(state->hLayoutSel, CB_GETCURSEL, 0, 0));
    if (sel < 0) return;
    int layoutIdx = static_cast<int>(SendMessageW(state->hLayoutSel, CB_GETITEMDATA, sel, 0));
    const auto* l = bijoy::core::GetLayoutByIndex(layoutIdx);
    if (!l) return;

    // Set Key dropdown
    int keyCount = static_cast<int>(SendMessageW(state->hLayoutKey, CB_GETCOUNT, 0, 0));
    for (int i = 0; i < keyCount; ++i) {
        int vk = static_cast<int>(SendMessageW(state->hLayoutKey, CB_GETITEMDATA, i, 0));
        if (vk == l->shortcut.keyCode) {
            SendMessageW(state->hLayoutKey, CB_SETCURSEL, i, 0);
            break;
        }
    }

    Button_SetCheck(state->hLayoutCtrl, l->shortcut.ctrl ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(state->hLayoutAlt, l->shortcut.alt ? BST_CHECKED : BST_UNCHECKED);
    Button_SetCheck(state->hLayoutShift, l->shortcut.shift ? BST_CHECKED : BST_UNCHECKED);
}

void SwitchTab(DialogState* state, int tabIndex) {
    if (!state) return;
    for (int i = 0; i < 4; ++i) {
        int show = (i == tabIndex) ? SW_SHOW : SW_HIDE;
        for (HWND hCtrl : state->tabControls[i]) {
            ShowWindow(hCtrl, show);
        }
    }
}

void ApplySettings(DialogState* state) {
    if (!state) return;

    // General tab
    state->currentOptions.autoStart = (Button_GetCheck(state->hAutoStart) == BST_CHECKED);
    state->currentOptions.trayMode = (Button_GetCheck(state->hTrayMode) == BST_CHECKED);
    state->currentOptions.showSplash = (Button_GetCheck(state->hSplash) == BST_CHECKED);
    state->currentOptions.layoutActivationMode = (Button_GetCheck(state->hWindowBinding) == BST_CHECKED);

    int defSel = static_cast<int>(SendMessageW(state->hDefaultLayout, CB_GETCURSEL, 0, 0));
    if (defSel >= 0) {
        state->currentOptions.defaultLayout = static_cast<int>(SendMessageW(state->hDefaultLayout, CB_GETITEMDATA, defSel, 0));
    }

    // Hotkeys tab
    int phonSel = static_cast<int>(SendMessageW(state->hPhoneticKey, CB_GETCURSEL, 0, 0));
    if (phonSel >= 0) {
        state->currentOptions.phoneticToggleKey = static_cast<int>(SendMessageW(state->hPhoneticKey, CB_GETITEMDATA, phonSel, 0));
    }
    state->currentOptions.phoneticToggleCtrl = (Button_GetCheck(state->hPhoneticCtrl) == BST_CHECKED);
    state->currentOptions.phoneticToggleAlt = (Button_GetCheck(state->hPhoneticAlt) == BST_CHECKED);
    state->currentOptions.phoneticToggleShift = (Button_GetCheck(state->hPhoneticShift) == BST_CHECKED);

    // Appearance tab
    state->currentOptions.overlayOpacity = static_cast<int>(SendMessageW(state->hOpacitySlider, TBM_GETPOS, 0, 0));
    state->currentOptions.alwaysOnTop = (Button_GetCheck(state->hTopmost) == BST_CHECKED);
    state->currentOptions.snapToTop = (Button_GetCheck(state->hSnapTop) == BST_CHECKED);

    // Save to Windows Registry
    bijoy::core::SaveStartupOptions(state->currentOptions);

    // Apply live changes
    bijoy::core::SetPhoneticToggleShortcut(
        state->currentOptions.phoneticToggleKey,
        state->currentOptions.phoneticToggleCtrl,
        state->currentOptions.phoneticToggleAlt,
        state->currentOptions.phoneticToggleShift);

    SetMainWindowOpacity(state->currentOptions.overlayOpacity);
    SetMainWindowAlwaysOnTop(state->currentOptions.alwaysOnTop);
    SetMainWindowSnapToTop(state->currentOptions.snapToTop);

    // Update initial snapshot so Cancel won't revert after Apply
    state->initialOptions = state->currentOptions;
}

LRESULT CALLBACK OptionsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
        case WM_CREATE: {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            state = reinterpret_cast<DialogState*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

            // Load initial options
            state->initialOptions = bijoy::core::LoadStartupOptions();
            state->currentOptions = state->initialOptions;

            // Create fonts
            state->hFontNormal = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            state->hFontBold = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            state->hFontTitle = CreateFontW(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            // 1. Tab Control
            state->hTab = CreateWindowExW(
                0, WC_TABCONTROLW, L"",
                WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                10, 10, kDialogWidth - 36, kDialogHeight - 88,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_TAB), cs->hInstance, nullptr);
            SendMessageW(state->hTab, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontNormal), TRUE);

            TCITEMW tie = {};
            tie.mask = TCIF_TEXT;
            tie.pszText = const_cast<LPWSTR>(L"General & Startup");
            TabCtrl_InsertItem(state->hTab, 0, &tie);
            tie.pszText = const_cast<LPWSTR>(L"Hotkeys & Shortcuts");
            TabCtrl_InsertItem(state->hTab, 1, &tie);
            tie.pszText = const_cast<LPWSTR>(L"Appearance & Overlay");
            TabCtrl_InsertItem(state->hTab, 2, &tie);
            tie.pszText = const_cast<LPWSTR>(L"About & Updates");
            TabCtrl_InsertItem(state->hTab, 3, &tie);

            // =================================================================
            // TAB 0: GENERAL & STARTUP
            // =================================================================
            HWND hGrp1 = CreateWindowExW(0, WC_BUTTONW, L"Startup & Windows Integration",
                WS_CHILD | BS_GROUPBOX, 22, 42, 470, 110, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[0].push_back(hGrp1);

            state->hAutoStart = CreateWindowExW(0, WC_BUTTONW, L"Start Omor Ekushe automatically on Windows boot",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 64, 440, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_AUTOSTART), cs->hInstance, nullptr);
            state->tabControls[0].push_back(state->hAutoStart);
            Button_SetCheck(state->hAutoStart, state->initialOptions.autoStart ? BST_CHECKED : BST_UNCHECKED);

            state->hTrayMode = CreateWindowExW(0, WC_BUTTONW, L"Start minimized to system tray (hide toolbar on launch)",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 90, 440, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_TRAYMODE), cs->hInstance, nullptr);
            state->tabControls[0].push_back(state->hTrayMode);
            Button_SetCheck(state->hTrayMode, state->initialOptions.trayMode ? BST_CHECKED : BST_UNCHECKED);

            state->hSplash = CreateWindowExW(0, WC_BUTTONW, L"Show splash screen during application launch",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 116, 440, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_SPLASH), cs->hInstance, nullptr);
            state->tabControls[0].push_back(state->hSplash);
            Button_SetCheck(state->hSplash, state->initialOptions.showSplash ? BST_CHECKED : BST_UNCHECKED);

            HWND hGrp2 = CreateWindowExW(0, WC_BUTTONW, L"Default Keyboard Layout",
                WS_CHILD | BS_GROUPBOX, 22, 160, 470, 85, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[0].push_back(hGrp2);

            HWND hLblDef = CreateWindowExW(0, WC_STATICW, L"Select layout to activate automatically on startup:",
                WS_CHILD, 36, 182, 440, 18, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[0].push_back(hLblDef);

            state->hDefaultLayout = CreateWindowExW(0, WC_COMBOBOXW, L"",
                WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 36, 204, 260, 200,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_DEFAULT_LAYOUT), cs->hInstance, nullptr);
            state->tabControls[0].push_back(state->hDefaultLayout);

            // Populate default layout dropdown
            int engIdx = static_cast<int>(SendMessageW(state->hDefaultLayout, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English (Pass-through)")));
            SendMessageW(state->hDefaultLayout, CB_SETITEMDATA, engIdx, static_cast<LPARAM>(-1));

            int targetSel = 0;
            if (state->initialOptions.defaultLayout == -1) targetSel = engIdx;

            for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
                const auto* l = bijoy::core::GetLayoutByIndex(i);
                if (l) {
                    int idx = static_cast<int>(SendMessageW(state->hDefaultLayout, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(l->name.c_str())));
                    SendMessageW(state->hDefaultLayout, CB_SETITEMDATA, idx, static_cast<LPARAM>(i));
                    if (i == state->initialOptions.defaultLayout) {
                        targetSel = idx;
                    }
                }
            }
            SendMessageW(state->hDefaultLayout, CB_SETCURSEL, targetSel, 0);

            HWND hGrp3 = CreateWindowExW(0, WC_BUTTONW, L"Multi-Window Typing Behavior",
                WS_CHILD | BS_GROUPBOX, 22, 255, 470, 135, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[0].push_back(hGrp3);

            state->hWindowBinding = CreateWindowExW(0, WC_BUTTONW, L"Per-window layout memory (remember layout per application)",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 278, 440, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_WINDOW_BINDING), cs->hInstance, nullptr);
            state->tabControls[0].push_back(state->hWindowBinding);
            Button_SetCheck(state->hWindowBinding, state->initialOptions.layoutActivationMode ? BST_CHECKED : BST_UNCHECKED);

            HWND hLblDesc = CreateWindowExW(0, WC_STATICW,
                L"When enabled, switching to Bangla in one window (e.g. Word) does not affect "
                L"other applications (e.g. browser or code editor).\r\n"
                L"When disabled, keyboard layout changes apply globally across the entire system.",
                WS_CHILD, 36, 305, 440, 75, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[0].push_back(hLblDesc);

            // =================================================================
            // TAB 1: HOTKEYS & SHORTCUTS
            // =================================================================
            HWND hGrpHot1 = CreateWindowExW(0, WC_BUTTONW, L"Quick Bangla / Phonetic Toggle Hotkey",
                WS_CHILD | BS_GROUPBOX, 22, 42, 470, 125, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hGrpHot1);

            HWND hLblPhon = CreateWindowExW(0, WC_STATICW, L"Global shortcut key to toggle between English and Phonetic Bangla:",
                WS_CHILD, 36, 64, 440, 18, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hLblPhon);

            HWND hLblPhonKey = CreateWindowExW(0, WC_STATICW, L"Key:", WS_CHILD, 36, 88, 30, 20, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hLblPhonKey);

            state->hPhoneticKey = CreateWindowExW(0, WC_COMBOBOXW, L"",
                WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 70, 86, 120, 200,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_PHONETIC_KEY), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hPhoneticKey);

            int phonSelIdx = 0;
            for (size_t i = 0; i < sizeof(kPhoneticKeys) / sizeof(kPhoneticKeys[0]); ++i) {
                int idx = static_cast<int>(SendMessageW(state->hPhoneticKey, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kPhoneticKeys[i].name)));
                SendMessageW(state->hPhoneticKey, CB_SETITEMDATA, idx, static_cast<LPARAM>(kPhoneticKeys[i].vkCode));
                if (kPhoneticKeys[i].vkCode == state->initialOptions.phoneticToggleKey) {
                    phonSelIdx = idx;
                }
            }
            SendMessageW(state->hPhoneticKey, CB_SETCURSEL, phonSelIdx, 0);

            state->hPhoneticCtrl = CreateWindowExW(0, WC_BUTTONW, L"Ctrl",
                WS_CHILD | BS_AUTOCHECKBOX, 205, 88, 50, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_PHONETIC_CTRL), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hPhoneticCtrl);
            Button_SetCheck(state->hPhoneticCtrl, state->initialOptions.phoneticToggleCtrl ? BST_CHECKED : BST_UNCHECKED);

            state->hPhoneticAlt = CreateWindowExW(0, WC_BUTTONW, L"Alt",
                WS_CHILD | BS_AUTOCHECKBOX, 260, 88, 50, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_PHONETIC_ALT), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hPhoneticAlt);
            Button_SetCheck(state->hPhoneticAlt, state->initialOptions.phoneticToggleAlt ? BST_CHECKED : BST_UNCHECKED);

            state->hPhoneticShift = CreateWindowExW(0, WC_BUTTONW, L"Shift",
                WS_CHILD | BS_AUTOCHECKBOX, 315, 88, 55, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_PHONETIC_SHIFT), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hPhoneticShift);
            Button_SetCheck(state->hPhoneticShift, state->initialOptions.phoneticToggleShift ? BST_CHECKED : BST_UNCHECKED);

            state->hPhoneticStatus = CreateWindowExW(0, WC_STATICW, L"",
                WS_CHILD, 36, 115, 440, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_PHONETIC_STATUS), cs->hInstance, nullptr);
            SendMessageW(state->hPhoneticStatus, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontBold), TRUE);
            state->tabControls[1].push_back(state->hPhoneticStatus);
            UpdatePhoneticStatusBadge(state);

            HWND hGrpHot2 = CreateWindowExW(0, WC_BUTTONW, L"Layout Switch Shortcuts",
                WS_CHILD | BS_GROUPBOX, 22, 175, 470, 215, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hGrpHot2);

            HWND hLblLayDesc = CreateWindowExW(0, WC_STATICW, L"Select a keyboard layout to customize its activation hotkey:",
                WS_CHILD, 36, 196, 440, 18, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hLblLayDesc);

            state->hLayoutSel = CreateWindowExW(0, WC_COMBOBOXW, L"",
                WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 36, 218, 170, 200,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_SEL), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutSel);

            for (int i = 0; i < bijoy::core::GetLayoutCount(); ++i) {
                const auto* l = bijoy::core::GetLayoutByIndex(i);
                if (l) {
                    int idx = static_cast<int>(SendMessageW(state->hLayoutSel, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(l->name.c_str())));
                    SendMessageW(state->hLayoutSel, CB_SETITEMDATA, idx, static_cast<LPARAM>(i));
                }
            }
            if (bijoy::core::GetLayoutCount() > 0) {
                SendMessageW(state->hLayoutSel, CB_SETCURSEL, 0, 0);
            }

            HWND hLblLayKey = CreateWindowExW(0, WC_STATICW, L"Key:", WS_CHILD, 215, 220, 30, 20, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[1].push_back(hLblLayKey);

            state->hLayoutKey = CreateWindowExW(0, WC_COMBOBOXW, L"",
                WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, 245, 218, 70, 200,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_KEY), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutKey);

            for (size_t i = 0; i < sizeof(kLayoutKeys) / sizeof(kLayoutKeys[0]); ++i) {
                int idx = static_cast<int>(SendMessageW(state->hLayoutKey, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(kLayoutKeys[i].name)));
                SendMessageW(state->hLayoutKey, CB_SETITEMDATA, idx, static_cast<LPARAM>(kLayoutKeys[i].vkCode));
            }

            state->hLayoutCtrl = CreateWindowExW(0, WC_BUTTONW, L"Ctrl",
                WS_CHILD | BS_AUTOCHECKBOX, 325, 220, 48, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_CTRL), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutCtrl);

            state->hLayoutAlt = CreateWindowExW(0, WC_BUTTONW, L"Alt",
                WS_CHILD | BS_AUTOCHECKBOX, 375, 220, 45, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_ALT), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutAlt);

            state->hLayoutShift = CreateWindowExW(0, WC_BUTTONW, L"Shift",
                WS_CHILD | BS_AUTOCHECKBOX, 422, 220, 52, 20, hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_SHIFT), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutShift);

            HWND hBtnAssign = CreateWindowExW(0, WC_BUTTONW, L"Assign Shortcut",
                WS_CHILD | BS_PUSHBUTTON, 36, 250, 130, 26,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_ASSIGN), cs->hInstance, nullptr);
            state->tabControls[1].push_back(hBtnAssign);

            HWND hBtnReset = CreateWindowExW(0, WC_BUTTONW, L"Reset to Defaults",
                WS_CHILD | BS_PUSHBUTTON, 175, 250, 130, 26,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_RESET), cs->hInstance, nullptr);
            state->tabControls[1].push_back(hBtnReset);

            state->hLayoutSummary = CreateWindowExW(0, WC_EDITW, L"",
                WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
                36, 285, 440, 95, hwnd, reinterpret_cast<HMENU>(IDC_OPT_LAYOUT_LIST), cs->hInstance, nullptr);
            state->tabControls[1].push_back(state->hLayoutSummary);

            SyncSelectedLayoutControls(state);
            RefreshLayoutShortcutsSummary(state);

            // =================================================================
            // TAB 2: APPEARANCE & OVERLAY
            // =================================================================
            HWND hGrpApp1 = CreateWindowExW(0, WC_BUTTONW, L"Toolbar Transparency & Opacity",
                WS_CHILD | BS_GROUPBOX, 22, 42, 470, 185, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[2].push_back(hGrpApp1);

            HWND hLblOpDesc = CreateWindowExW(0, WC_STATICW,
                L"Adjust the opacity of the floating toolbar overlay (instant live preview):",
                WS_CHILD, 36, 64, 440, 18, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[2].push_back(hLblOpDesc);

            state->hOpacitySlider = CreateWindowExW(
                0, TRACKBAR_CLASSW, L"",
                WS_CHILD | TBS_AUTOTICKS | TBS_HORZ | WS_TABSTOP,
                36, 88, 330, 32,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_OPACITY_SLIDER), cs->hInstance, nullptr);
            state->tabControls[2].push_back(state->hOpacitySlider);

            SendMessageW(state->hOpacitySlider, TBM_SETRANGE, TRUE, MAKELPARAM(20, 100));
            SendMessageW(state->hOpacitySlider, TBM_SETTICFREQ, 10, 0);
            SendMessageW(state->hOpacitySlider, TBM_SETPOS, TRUE, state->initialOptions.overlayOpacity);

            state->hOpacityLabel = CreateWindowExW(0, WC_STATICW, L"100% (Solid)",
                WS_CHILD, 375, 94, 110, 20,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_OPACITY_LABEL), cs->hInstance, nullptr);
            SendMessageW(state->hOpacityLabel, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontBold), TRUE);
            state->tabControls[2].push_back(state->hOpacityLabel);

            wchar_t opText[64] = {};
            if (state->initialOptions.overlayOpacity >= 100) {
                wcscpy_s(opText, L"100% (Solid)");
            } else {
                swprintf_s(opText, L"%d%% (Translucent)", state->initialOptions.overlayOpacity);
            }
            SetWindowTextW(state->hOpacityLabel, opText);

            HWND hBtn50 = CreateWindowExW(0, WC_BUTTONW, L"50% Opacity",
                WS_CHILD | BS_PUSHBUTTON, 36, 128, 95, 26,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_OPACITY_50), cs->hInstance, nullptr);
            state->tabControls[2].push_back(hBtn50);

            HWND hBtn75 = CreateWindowExW(0, WC_BUTTONW, L"75% Opacity",
                WS_CHILD | BS_PUSHBUTTON, 140, 128, 95, 26,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_OPACITY_75), cs->hInstance, nullptr);
            state->tabControls[2].push_back(hBtn75);

            HWND hBtn100 = CreateWindowExW(0, WC_BUTTONW, L"100% (Solid)",
                WS_CHILD | BS_PUSHBUTTON, 244, 128, 95, 26,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_OPACITY_100), cs->hInstance, nullptr);
            state->tabControls[2].push_back(hBtn100);

            HWND hLblOpHint = CreateWindowExW(0, WC_STATICW,
                L"Lowering the opacity makes the toolbar semi-transparent, allowing you to see "
                L"documents and applications underneath while typing.",
                WS_CHILD, 36, 164, 440, 45, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[2].push_back(hLblOpHint);

            HWND hGrpApp2 = CreateWindowExW(0, WC_BUTTONW, L"Toolbar Docking & Positioning",
                WS_CHILD | BS_GROUPBOX, 22, 235, 470, 155, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[2].push_back(hGrpApp2);

            state->hTopmost = CreateWindowExW(0, WC_BUTTONW, L"Always keep toolbar on top of other windows (Topmost)",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 258, 440, 20,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_TOPMOST), cs->hInstance, nullptr);
            state->tabControls[2].push_back(state->hTopmost);
            Button_SetCheck(state->hTopmost, state->initialOptions.alwaysOnTop ? BST_CHECKED : BST_UNCHECKED);

            state->hSnapTop = CreateWindowExW(0, WC_BUTTONW, L"Snap toolbar to top edge of the screen",
                WS_CHILD | BS_AUTOCHECKBOX, 36, 284, 440, 20,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_SNAP_TOP), cs->hInstance, nullptr);
            state->tabControls[2].push_back(state->hSnapTop);
            Button_SetCheck(state->hSnapTop, state->initialOptions.snapToTop ? BST_CHECKED : BST_UNCHECKED);

            HWND hBtnResetPos = CreateWindowExW(0, WC_BUTTONW, L"Reset Toolbar Position (Center-Top)",
                WS_CHILD | BS_PUSHBUTTON, 36, 314, 250, 28,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_RESET_POS), cs->hInstance, nullptr);
            state->tabControls[2].push_back(hBtnResetPos);

            HWND hLblPosHint = CreateWindowExW(0, WC_STATICW,
                L"Tip: You can drag the toolbar horizontally across the top edge of your monitor using the mouse.",
                WS_CHILD, 36, 350, 440, 25, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[2].push_back(hLblPosHint);

            // =================================================================
            // TAB 3: ABOUT & UPDATES
            // =================================================================
            HWND hGrpAbt1 = CreateWindowExW(0, WC_BUTTONW, L"About Omor Ekushe",
                WS_CHILD | BS_GROUPBOX, 22, 42, 470, 195, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[3].push_back(hGrpAbt1);

            HWND hAbtTitle = CreateWindowExW(0, WC_STATICW, L"Omor Ekushe — অমর একুশে (v1.1.0)",
                WS_CHILD, 36, 66, 440, 24, hwnd, nullptr, cs->hInstance, nullptr);
            SendMessageW(hAbtTitle, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontTitle), TRUE);
            state->tabControls[3].push_back(hAbtTitle);

            HWND hAbtSub = CreateWindowExW(0, WC_STATICW,
                L"Modern Native Bangla Typing Engine for Windows (Win32 / C++17)\r\n"
                L"Engineered with zero runtime overhead and instant responsiveness.",
                WS_CHILD, 36, 94, 440, 34, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[3].push_back(hAbtSub);

            HWND hAbtFeatures = CreateWindowExW(0, WC_STATICW,
                L"Key Features:\r\n"
                L" • Avro-compatible phonetic transliteration with dynamic backspacing\r\n"
                L" • Unicode & ANSI Classic layout support (Bijoy Classic/Unicode)\r\n"
                L" • Real-time customizable hotkeys and translucent toolbar overlay\r\n"
                L" • Built-in Layout Editor & Spelling Checker core",
                WS_CHILD, 36, 132, 440, 70, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[3].push_back(hAbtFeatures);

            HWND hAbtAuthor = CreateWindowExW(0, WC_STATICW,
                L"Developed by Sabbir Legacy. Released under the MIT License.",
                WS_CHILD, 36, 206, 440, 18, hwnd, nullptr, cs->hInstance, nullptr);
            SendMessageW(hAbtAuthor, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontBold), TRUE);
            state->tabControls[3].push_back(hAbtAuthor);

            HWND hGrpAbt2 = CreateWindowExW(0, WC_BUTTONW, L"Tools & Online Resources",
                WS_CHILD | BS_GROUPBOX, 22, 245, 470, 145, hwnd, nullptr, cs->hInstance, nullptr);
            state->tabControls[3].push_back(hGrpAbt2);

            HWND hBtnUpdate = CreateWindowExW(0, WC_BUTTONW, L"Check for Updates...",
                WS_CHILD | BS_PUSHBUTTON, 36, 272, 190, 30,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_UPDATE), cs->hInstance, nullptr);
            state->tabControls[3].push_back(hBtnUpdate);

            HWND hBtnGithub = CreateWindowExW(0, WC_BUTTONW, L"Visit Project on GitHub",
                WS_CHILD | BS_PUSHBUTTON, 245, 272, 190, 30,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_GITHUB), cs->hInstance, nullptr);
            state->tabControls[3].push_back(hBtnGithub);

            HWND hBtnLicense = CreateWindowExW(0, WC_BUTTONW, L"View MIT License",
                WS_CHILD | BS_PUSHBUTTON, 36, 314, 190, 30,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_LICENSE), cs->hInstance, nullptr);
            state->tabControls[3].push_back(hBtnLicense);

            HWND hBtnEditor = CreateWindowExW(0, WC_BUTTONW, L"Launch Layout Editor",
                WS_CHILD | BS_PUSHBUTTON, 245, 314, 190, 30,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_EDITOR), cs->hInstance, nullptr);
            state->tabControls[3].push_back(hBtnEditor);

            // =================================================================
            // BOTTOM ACTION BUTTONS
            // =================================================================
            HWND hBtnOk = CreateWindowExW(0, WC_BUTTONW, L"OK",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                kDialogWidth - 275, kDialogHeight - 65, 80, 28,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_OK), cs->hInstance, nullptr);
            SendMessageW(hBtnOk, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontNormal), TRUE);

            HWND hBtnCancel = CreateWindowExW(0, WC_BUTTONW, L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                kDialogWidth - 188, kDialogHeight - 65, 80, 28,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_CANCEL), cs->hInstance, nullptr);
            SendMessageW(hBtnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontNormal), TRUE);

            HWND hBtnApply = CreateWindowExW(0, WC_BUTTONW, L"Apply",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                kDialogWidth - 100, kDialogHeight - 65, 78, 28,
                hwnd, reinterpret_cast<HMENU>(IDC_OPT_BTN_APPLY), cs->hInstance, nullptr);
            SendMessageW(hBtnApply, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontNormal), TRUE);

            // Set font for all tab controls
            for (int t = 0; t < 4; ++t) {
                for (HWND hCtrl : state->tabControls[t]) {
                    SendMessageW(hCtrl, WM_SETFONT, reinterpret_cast<WPARAM>(state->hFontNormal), TRUE);
                }
            }

            // Show initial tab
            SwitchTab(state, 0);
            return 0;
        }

        case WM_NOTIFY: {
            NMHDR* nm = reinterpret_cast<NMHDR*>(lParam);
            if (nm && nm->idFrom == IDC_OPT_TAB && nm->code == TCN_SELCHANGE) {
                int curSel = TabCtrl_GetCurSel(state->hTab);
                SwitchTab(state, curSel);
            }
            return 0;
        }

        case WM_HSCROLL: {
            if (reinterpret_cast<HWND>(lParam) == state->hOpacitySlider) {
                int pos = static_cast<int>(SendMessageW(state->hOpacitySlider, TBM_GETPOS, 0, 0));
                wchar_t buf[64] = {};
                if (pos >= 100) {
                    wcscpy_s(buf, L"100% (Solid)");
                } else {
                    swprintf_s(buf, L"%d%% (Translucent)", pos);
                }
                SetWindowTextW(state->hOpacityLabel, buf);
                // Live preview of opacity
                SetMainWindowOpacity(pos);
            }
            return 0;
        }

        case WM_COMMAND: {
            const UINT id = LOWORD(wParam);
            const UINT code = HIWORD(wParam);

            // Quick phonetic toggle checkboxes/combo change
            if (id == IDC_OPT_PHONETIC_KEY || id == IDC_OPT_PHONETIC_CTRL ||
                id == IDC_OPT_PHONETIC_ALT || id == IDC_OPT_PHONETIC_SHIFT) {
                UpdatePhoneticStatusBadge(state);
                return 0;
            }

            // Layout combo selection change
            if (id == IDC_OPT_LAYOUT_SEL && code == CBN_SELCHANGE) {
                SyncSelectedLayoutControls(state);
                return 0;
            }

            // Assign layout shortcut button
            if (id == IDC_OPT_LAYOUT_ASSIGN) {
                int sel = static_cast<int>(SendMessageW(state->hLayoutSel, CB_GETCURSEL, 0, 0));
                if (sel >= 0) {
                    int layoutIdx = static_cast<int>(SendMessageW(state->hLayoutSel, CB_GETITEMDATA, sel, 0));
                    auto* l = bijoy::core::GetLayoutByIndex(layoutIdx);
                    if (l) {
                        int keySel = static_cast<int>(SendMessageW(state->hLayoutKey, CB_GETCURSEL, 0, 0));
                        if (keySel >= 0) {
                            l->shortcut.keyCode = static_cast<int>(SendMessageW(state->hLayoutKey, CB_GETITEMDATA, keySel, 0));
                        }
                        l->shortcut.ctrl = (Button_GetCheck(state->hLayoutCtrl) == BST_CHECKED);
                        l->shortcut.alt = (Button_GetCheck(state->hLayoutAlt) == BST_CHECKED);
                        l->shortcut.shift = (Button_GetCheck(state->hLayoutShift) == BST_CHECKED);

                        bijoy::core::SaveCustomLayoutShortcut(l->name, l->shortcut);
                        RefreshLayoutShortcutsSummary(state);
                        MessageBoxW(hwnd, L"Layout shortcut updated successfully!", L"Settings", MB_OK | MB_ICONINFORMATION);
                    }
                }
                return 0;
            }

            // Reset layout shortcuts button
            if (id == IDC_OPT_LAYOUT_RESET) {
                if (MessageBoxW(hwnd, L"Reset all layout shortcuts to their default XML settings?", L"Confirm Reset", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    bijoy::core::ResetCustomLayoutShortcuts();
                    // Reload layouts
                    const std::wstring appDir = bijoy::core::GetAppDirectory();
                    bijoy::core::FindLayouts(bijoy::core::g_layouts, appDir);
                    SyncSelectedLayoutControls(state);
                    RefreshLayoutShortcutsSummary(state);
                }
                return 0;
            }

            // Opacity preset buttons
            if (id == IDC_OPT_OPACITY_50) {
                SendMessageW(state->hOpacitySlider, TBM_SETPOS, TRUE, 50);
                SetWindowTextW(state->hOpacityLabel, L"50% (Translucent)");
                SetMainWindowOpacity(50);
                return 0;
            }
            if (id == IDC_OPT_OPACITY_75) {
                SendMessageW(state->hOpacitySlider, TBM_SETPOS, TRUE, 75);
                SetWindowTextW(state->hOpacityLabel, L"75% (Translucent)");
                SetMainWindowOpacity(75);
                return 0;
            }
            if (id == IDC_OPT_OPACITY_100) {
                SendMessageW(state->hOpacitySlider, TBM_SETPOS, TRUE, 100);
                SetWindowTextW(state->hOpacityLabel, L"100% (Solid)");
                SetMainWindowOpacity(100);
                return 0;
            }

            // Topmost live toggle
            if (id == IDC_OPT_TOPMOST) {
                bool tm = (Button_GetCheck(state->hTopmost) == BST_CHECKED);
                SetMainWindowAlwaysOnTop(tm);
                return 0;
            }

            // Reset toolbar position button
            if (id == IDC_OPT_RESET_POS) {
                ResetMainWindowPosition();
                return 0;
            }

            // About tab buttons
            if (id == IDC_OPT_BTN_GITHUB) {
                ShellExecuteW(nullptr, L"open", L"https://github.com/sabbir28/OmorEkushe", nullptr, nullptr, SW_SHOWNORMAL);
                return 0;
            }
            if (id == IDC_OPT_BTN_LICENSE) {
                if (state->parent) {
                    PostMessageW(state->parent, WM_COMMAND, MAKEWPARAM(IDM_LICENSE, 0), 0);
                }
                return 0;
            }
            if (id == IDC_OPT_BTN_UPDATE) {
                if (state->parent) {
                    PostMessageW(state->parent, WM_COMMAND, MAKEWPARAM(IDM_UPDATE, 0), 0);
                }
                return 0;
            }
            if (id == IDC_OPT_BTN_EDITOR) {
                const std::wstring appDir = bijoy::core::GetAppDirectory();
                const std::wstring editorPath = BuildPath(appDir, L"LayoutEditor.exe");
                HINSTANCE result = ShellExecuteW(hwnd, L"open", editorPath.c_str(), nullptr, nullptr, SW_SHOW);
                if (reinterpret_cast<INT_PTR>(result) <= 32) {
                    MessageBoxW(hwnd, L"Failed to open Layout Editor. Ensure LayoutEditor.exe is present.", L"Omor Ekushe", MB_OK | MB_ICONERROR);
                }
                return 0;
            }

            // Dialog bottom buttons
            if (id == IDC_OPT_BTN_APPLY) {
                ApplySettings(state);
                return 0;
            }
            if (id == IDC_OPT_BTN_OK) {
                ApplySettings(state);
                DestroyWindow(hwnd);
                return 0;
            }
            if (id == IDC_OPT_BTN_CANCEL || id == IDCANCEL) {
                // Revert any live preview adjustments
                SetMainWindowOpacity(state->initialOptions.overlayOpacity);
                SetMainWindowAlwaysOnTop(state->initialOptions.alwaysOnTop);
                SetMainWindowSnapToTop(state->initialOptions.snapToTop);
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            // Cancel / close: revert live preview
            SetMainWindowOpacity(state->initialOptions.overlayOpacity);
            SetMainWindowAlwaysOnTop(state->initialOptions.alwaysOnTop);
            SetMainWindowSnapToTop(state->initialOptions.snapToTop);
            DestroyWindow(hwnd);
            return 0;
        }

        case WM_DESTROY: {
            if (state->parent && IsWindow(state->parent)) {
                EnableWindow(state->parent, TRUE);
                SetForegroundWindow(state->parent);
            }

            if (state->hFontNormal) DeleteObject(state->hFontNormal);
            if (state->hFontBold) DeleteObject(state->hFontBold);
            if (state->hFontTitle) DeleteObject(state->hFontTitle);

            delete state;
            g_settingsDialog = nullptr;
            return 0;
        }

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

void ShowOptionsDialog(HWND parent) {
    if (g_settingsDialog && IsWindow(g_settingsDialog)) {
        SetForegroundWindow(g_settingsDialog);
        SetFocus(g_settingsDialog);
        return;
    }

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = OptionsDialogProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kOptionsDialogClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP));

    RegisterClassExW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - kDialogWidth) / 2;
    int posY = (screenH - kDialogHeight) / 2;

    auto* state = new DialogState();
    state->parent = parent;

    if (parent && IsWindow(parent)) {
        EnableWindow(parent, FALSE);
    }

    g_settingsDialog = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        kOptionsDialogClass,
        L"Omor Ekushe — Settings & Configuration",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_VISIBLE,
        posX, posY, kDialogWidth, kDialogHeight,
        parent, nullptr, hInstance, state);

    if (g_settingsDialog) {
        ShowWindow(g_settingsDialog, SW_SHOW);
        UpdateWindow(g_settingsDialog);
        SetForegroundWindow(g_settingsDialog);
    } else {
        if (parent && IsWindow(parent)) {
            EnableWindow(parent, TRUE);
        }
        delete state;
    }
}

} // namespace bijoy::platform::windows

