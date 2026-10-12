// =============================================================================
// File   : startup_options.cpp
// Module : core (Persistent Startup Options & Configuration)
// Purpose: Reads and writes user startup preferences to the Windows Registry
//          under HKCU\SOFTWARE\BijoyEkushe\Options. Settings include:
//            - DefaultLayout: last-used keyboard layout index
//            - Position / Top: saved main window screen coordinates
//            - LAM: layout activation mode toggle (per-window binding)
//            - TrayMode: whether to start minimized to system tray
//            - ApplicationMode: application visibility mode
//            - OverlayOpacity: floating toolbar alpha (20 - 100%)
//            - AlwaysOnTop: keep toolbar topmost over other windows
//            - SnapToTop: enforce toolbar docking to top edge
//            - ShowSplash: show startup splash window
//            - PhoneticKey / Ctrl / Alt / Shift: custom hotkey for Bangla
//            - AutoStart: Windows Run registry entry integration
//            - Custom layout shortcuts under HKCU\SOFTWARE\BijoyEkushe\Options\Shortcuts
// =============================================================================

#include "core/startup_options.h"

#include <windows.h>
#include <string>

namespace bijoy::core {
    namespace {

        constexpr wchar_t kOptionsKey[] = L"SOFTWARE\\BijoyEkushe\\Options";
        constexpr wchar_t kShortcutsSubKey[] = L"SOFTWARE\\BijoyEkushe\\Options\\Shortcuts";
        constexpr wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        constexpr wchar_t kRunValueName[] = L"OmorEkushe";

        int ReadDwordValue(HKEY key, const wchar_t* valueName, int fallback) {
            DWORD data = 0;
            DWORD dataSize = sizeof(data);
            DWORD valueType = 0;
            const LONG status = RegGetValueW(key, nullptr, valueName, RRF_RT_REG_DWORD, &valueType, &data, &dataSize);
            if (status != ERROR_SUCCESS) {
                return fallback;
            }
            return static_cast<int>(data);
        }

        bool ReadBoolValue(HKEY key, const wchar_t* valueName, bool fallback) {
            return ReadDwordValue(key, valueName, fallback ? 1 : 0) != 0;
        }

    } // namespace

    bool IsAutoStartEnabled() {
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return false;
        }

        DWORD type = 0;
        DWORD size = 0;
        const LONG status = RegQueryValueExW(key, kRunValueName, nullptr, &type, nullptr, &size);
        RegCloseKey(key);
        return (status == ERROR_SUCCESS);
    }

    bool SetAutoStartEnabled(bool enable) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) {
            return false;
        }

        bool result = false;
        if (enable) {
            wchar_t exePath[MAX_PATH] = {};
            if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) > 0) {
                std::wstring cmd = L"\"" + std::wstring(exePath) + L"\"";
                const LONG status = RegSetValueExW(
                    key,
                    kRunValueName,
                    0,
                    REG_SZ,
                    reinterpret_cast<const BYTE*>(cmd.c_str()),
                    static_cast<DWORD>((cmd.size() + 1) * sizeof(wchar_t)));
                result = (status == ERROR_SUCCESS);
            }
        } else {
            const LONG status = RegDeleteValueW(key, kRunValueName);
            result = (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND);
        }

        RegCloseKey(key);
        return result;
    }

    StartupOptions LoadStartupOptions() {
        StartupOptions options;
        options.autoStart = IsAutoStartEnabled();

        HKEY key = nullptr;
        const LONG openStatus = RegOpenKeyExW(HKEY_CURRENT_USER, kOptionsKey, 0, KEY_READ, &key);
        if (openStatus != ERROR_SUCCESS) {
            return options;
        }

        options.defaultLayout = ReadDwordValue(key, L"DefaultLayout", options.defaultLayout);
        options.mainWindowLeft = ReadDwordValue(key, L"Position", options.mainWindowLeft);
        options.mainWindowTop = ReadDwordValue(key, L"Top", options.mainWindowTop);
        options.layoutActivationMode = ReadBoolValue(key, L"LAM", options.layoutActivationMode);
        options.trayMode = ReadBoolValue(key, L"TrayMode", options.trayMode);
        options.applicationMode = ReadDwordValue(key, L"ApplicationMode", options.applicationMode);

        options.overlayOpacity = ReadDwordValue(key, L"OverlayOpacity", options.overlayOpacity);
        if (options.overlayOpacity < 20) options.overlayOpacity = 20;
        if (options.overlayOpacity > 100) options.overlayOpacity = 100;

        options.alwaysOnTop = ReadBoolValue(key, L"AlwaysOnTop", options.alwaysOnTop);
        options.snapToTop = ReadBoolValue(key, L"SnapToTop", options.snapToTop);
        options.showSplash = ReadBoolValue(key, L"ShowSplash", options.showSplash);

        options.phoneticToggleKey = ReadDwordValue(key, L"PhoneticKey", options.phoneticToggleKey);
        options.phoneticToggleCtrl = ReadBoolValue(key, L"PhoneticCtrl", options.phoneticToggleCtrl);
        options.phoneticToggleAlt = ReadBoolValue(key, L"PhoneticAlt", options.phoneticToggleAlt);
        options.phoneticToggleShift = ReadBoolValue(key, L"PhoneticShift", options.phoneticToggleShift);

        RegCloseKey(key);
        return options;
    }

    bool SaveStartupOptions(const StartupOptions& options) {
        SetAutoStartEnabled(options.autoStart);

        HKEY key = nullptr;
        DWORD disposition = 0;
        const LONG createStatus = RegCreateKeyExW(
                HKEY_CURRENT_USER,
                kOptionsKey,
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_WRITE,
                nullptr,
                &key,
                &disposition);
        (void)disposition;

        if (createStatus != ERROR_SUCCESS) {
            return false;
        }

        const auto writeDword = [key](const wchar_t* valueName, DWORD value) {
            return RegSetValueExW(
                    key,
                    valueName,
                    0,
                    REG_DWORD,
                    reinterpret_cast<const BYTE*>(&value),
                    sizeof(value)) == ERROR_SUCCESS;
        };

        const bool success =
                writeDword(L"DefaultLayout", static_cast<DWORD>(options.defaultLayout)) &&
                writeDword(L"Position", static_cast<DWORD>(options.mainWindowLeft)) &&
                writeDword(L"Top", static_cast<DWORD>(options.mainWindowTop)) &&
                writeDword(L"LAM", options.layoutActivationMode ? 1U : 0U) &&
                writeDword(L"TrayMode", options.trayMode ? 1U : 0U) &&
                writeDword(L"ApplicationMode", static_cast<DWORD>(options.applicationMode)) &&
                writeDword(L"OverlayOpacity", static_cast<DWORD>(options.overlayOpacity)) &&
                writeDword(L"AlwaysOnTop", options.alwaysOnTop ? 1U : 0U) &&
                writeDword(L"SnapToTop", options.snapToTop ? 1U : 0U) &&
                writeDword(L"ShowSplash", options.showSplash ? 1U : 0U) &&
                writeDword(L"PhoneticKey", static_cast<DWORD>(options.phoneticToggleKey)) &&
                writeDword(L"PhoneticCtrl", options.phoneticToggleCtrl ? 1U : 0U) &&
                writeDword(L"PhoneticAlt", options.phoneticToggleAlt ? 1U : 0U) &&
                writeDword(L"PhoneticShift", options.phoneticToggleShift ? 1U : 0U);

        RegCloseKey(key);
        return success;
    }

    bool LoadCustomLayoutShortcut(const std::wstring& layoutName, LayoutShortcut& shortcut) {
        if (layoutName.empty()) return false;

        const std::wstring subKeyPath = std::wstring(kShortcutsSubKey) + L"\\" + layoutName;
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, subKeyPath.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return false;
        }

        shortcut.keyCode = ReadDwordValue(key, L"KeyCode", shortcut.keyCode);
        shortcut.ctrl = ReadBoolValue(key, L"Ctrl", shortcut.ctrl);
        shortcut.alt = ReadBoolValue(key, L"Alt", shortcut.alt);
        shortcut.shift = ReadBoolValue(key, L"Shift", shortcut.shift);

        RegCloseKey(key);
        return true;
    }

    bool SaveCustomLayoutShortcut(const std::wstring& layoutName, const LayoutShortcut& shortcut) {
        if (layoutName.empty()) return false;

        const std::wstring subKeyPath = std::wstring(kShortcutsSubKey) + L"\\" + layoutName;
        HKEY key = nullptr;
        DWORD disposition = 0;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, subKeyPath.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, &disposition) != ERROR_SUCCESS) {
            return false;
        }

        const auto writeDword = [key](const wchar_t* valueName, DWORD value) {
            return RegSetValueExW(key, valueName, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value)) == ERROR_SUCCESS;
        };

        const bool success =
            writeDword(L"KeyCode", static_cast<DWORD>(shortcut.keyCode)) &&
            writeDword(L"Ctrl", shortcut.ctrl ? 1U : 0U) &&
            writeDword(L"Alt", shortcut.alt ? 1U : 0U) &&
            writeDword(L"Shift", shortcut.shift ? 1U : 0U);

        RegCloseKey(key);
        return success;
    }

    bool DeleteCustomLayoutShortcut(const std::wstring& layoutName) {
        if (layoutName.empty()) return false;
        const std::wstring subKeyPath = std::wstring(kShortcutsSubKey) + L"\\" + layoutName;
        return RegDeleteKeyW(HKEY_CURRENT_USER, subKeyPath.c_str()) == ERROR_SUCCESS;
    }

    void ResetCustomLayoutShortcuts() {
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kShortcutsSubKey, 0, KEY_READ | KEY_WRITE, &key) == ERROR_SUCCESS) {
            wchar_t subKeyName[256] = {};
            DWORD subKeyLen = 256;
            while (RegEnumKeyExW(key, 0, subKeyName, &subKeyLen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
                RegDeleteKeyW(key, subKeyName);
                subKeyLen = 256;
            }
            RegCloseKey(key);
        }
        RegDeleteKeyW(HKEY_CURRENT_USER, kShortcutsSubKey);
    }

} // namespace bijoy::core

