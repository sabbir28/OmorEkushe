// =============================================================================
// File   : keyboard_hook_service.cpp
// Module : core (Global Keyboard Hook)
// Purpose: Installs and manages a system-wide low-level keyboard hook
//          (WH_KEYBOARD_LL / hook ID 13). Intercepts all keyboard events
//          before they reach any application and translates them through
//          the active Layout's key-mapping table.
//
//          Hook behavior:
//            - Shortcut detection: Ctrl/Alt/Shift + KeyCode combos activate/
//              deactivate specific layouts per foreground window.
//            - Key remapping: When a layout is active, physical key presses
//              are consumed and replaced with Unicode character output via
//              SendInput(KEYEVENTF_UNICODE).
//
//          Safety: The hook callback is wrapped in a catch-all exception
//          barrier to prevent exception propagation into the OS hook dispatcher,
//          which would cause Windows to silently remove the hook.
// =============================================================================

#include "core/keyboard_hook_service.h"

#include "core/app_state.h"
#include "core/phonetic_engine.h"
#include "core/window_layout_binding.h"
#include "platform/windows/main_window/main_window_layout.h"
#include "platform/windows/native_input.h"

#include <string>
#include <windows.h>

namespace bijoy::core {

    namespace {

        struct KBDLLHOOKSTRUCT_LOCAL {
            DWORD vkCode;
            DWORD scanCode;
            DWORD flags;
            DWORD time;
            ULONG_PTR dwExtraInfo;
        };

        HHOOK g_hook = nullptr;
        HHOOK g_mouseHook = nullptr;
        bool g_layoutsReady = false;

        bool IsKeyPressed(int vk) {
            return (GetAsyncKeyState(vk) & 0x8000) != 0;
        }

        bool ProcessHookedEvent(const KBDLLHOOKSTRUCT_LOCAL* hs, WPARAM wParam) {
            const bool ctrl = IsKeyPressed(VK_CONTROL);
            const bool alt = IsKeyPressed(VK_MENU);
            const bool shift = IsKeyPressed(VK_SHIFT);
            const HWND foregroundWindow = GetForegroundWindow();

            // 1. Check F12 toggle for phonetic layout
            if (!ctrl && !alt && !shift && hs->vkCode == VK_F12) {
                int phoneticIndex = -1;
                for (int i = 0; i < GetLayoutCount(); ++i) {
                    Layout* l = GetLayoutByIndex(i);
                    if (l && l->isPhonetic) {
                        phoneticIndex = i;
                        break;
                    }
                }

                if (phoneticIndex >= 0) {
                    if (GetCurrentLayoutIndex() == phoneticIndex) {
                        RemoveWindowLayoutBinding(foregroundWindow);
                        bijoy::platform::windows::SelectLayout(-1);
                    } else {
                        Layout* pLayout = GetLayoutByIndex(phoneticIndex);
                        AddWindowLayoutBinding(foregroundWindow, pLayout);
                        bijoy::platform::windows::SelectLayout(phoneticIndex);
                    }
                    g_phoneticEngine.Reset();
                    return true;
                }
            }

            // 2. Check layout-specific shortcuts (e.g. Ctrl+Alt+B, Ctrl+Alt+V)
            for (int i = 0; i < GetLayoutCount(); ++i) {
                Layout* layout = GetLayoutByIndex(i);
                if (!layout) {
                    continue;
                }

                if (ctrl == layout->shortcut.ctrl && alt == layout->shortcut.alt &&
                    shift == layout->shortcut.shift && static_cast<DWORD>(layout->shortcut.keyCode) == hs->vkCode) {
                    if (g_comLayoutSelectedIndex == i + 1) {
                        RemoveWindowLayoutBinding(foregroundWindow);
                        bijoy::platform::windows::SelectLayout(-1);
                    } else {
                        AddWindowLayoutBinding(foregroundWindow, layout);
                        bijoy::platform::windows::SelectLayout(i);
                    }
                    g_phoneticEngine.Reset();
                    return true;
                }
            }

            // 3. Process key under active layout
            Layout* activeLayout = GetCurrentLayout();
            if (!activeLayout) {
                return false;
            }

            // 4. Phonetic transliteration mode
            if (activeLayout->isPhonetic) {
                return g_phoneticEngine.ProcessKey(
                    hs->vkCode,
                    (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN),
                    (wParam == WM_SYSKEYDOWN),
                    ctrl,
                    alt,
                    shift,
                    foregroundWindow);
            }

            // 5. Fixed layout key remapping (Bijoy Classic / Unicode)
            if (!ctrl && !alt) {
                auto it = activeLayout->key.find(static_cast<int>(hs->vkCode));
                if (it != activeLayout->key.end()) {
                    const std::wstring& output = shift ? it->second.shift : it->second.normal;
                    if (!output.empty()) {
                        for (wchar_t c : output) {
                            bijoy::platform::windows::DoKeyboard(
                                    KEYEVENTF_UNICODE,
                                    static_cast<int>(c));
                            bijoy::platform::windows::DoKeyboard(
                                    KEYEVENTF_KEYUP | KEYEVENTF_UNICODE,
                                    static_cast<int>(c));
                        }
                        return true;
                    }
                }
            }

            return false;
        }

        LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
            try {
                if (nCode == HC_ACTION && g_layoutsReady && lParam != 0) {
                    auto* hs = reinterpret_cast<KBDLLHOOKSTRUCT_LOCAL*>(lParam);

                    // Ignore synthetic/injected inputs
                    if ((hs->flags & LLKHF_INJECTED) != 0 || hs->dwExtraInfo == OMOR_EKUSHE_EXTRA_INFO) {
                        return CallNextHookEx(g_hook, nCode, wParam, lParam);
                    }

                    if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && ProcessHookedEvent(hs, wParam)) {
                        return 1;
                    }
                }
            } catch (...) {
                // Catch-all exception barrier: prevent exception propagation into OS hook dispatcher
            }
            return CallNextHookEx(g_hook, nCode, wParam, lParam);
        }

        LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
            try {
                if (nCode == HC_ACTION) {
                    if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
                        g_phoneticEngine.Reset();
                    }
                }
            } catch (...) {
                // Exception barrier
            }
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        }

    } // namespace

    bool InstallKeyboardHook(HINSTANCE hInstance) {
        g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
        g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
        return g_hook != nullptr;
    }

    void UninstallKeyboardHook() {
        if (g_hook) {
            UnhookWindowsHookEx(g_hook);
            g_hook = nullptr;
        }
        if (g_mouseHook) {
            UnhookWindowsHookEx(g_mouseHook);
            g_mouseHook = nullptr;
        }
    }

    void SetLayoutsReady(bool ready) {
        g_layoutsReady = ready;
    }

} // namespace bijoy::core
