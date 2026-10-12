// =============================================================================
// File   : system_utils.cpp
// Module : utils (System & Registration Utilities)
// Purpose: Cross-cutting utility functions for Omor Ekushe.
//          Cross-platform support for Windows, Linux, and macOS.
// =============================================================================

#include "utils/system_utils.h"
#include "error/crash_handler.h"

#include <algorithm>
#include <cstdlib>
#include <cwctype>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace bijoy::utils {

// -----------------------------------------------------------------------------
// Global static password seed
// Key : RN28 - T29S - K1XM - J6XY - LK24
// -----------------------------------------------------------------------------
    std::wstring Password = L"RN28T29SK1XMJ6XYLK24";

// -----------------------------------------------------------------------------
// Ensures a filesystem path always ends with a backslash / slash
// -----------------------------------------------------------------------------
    std::wstring EnsureTrailingBackslash(const std::wstring& path) {
        if (path.empty()) {
            return path;
        }

        std::wstring normalized = path;
#if defined(_WIN32)
        if (normalized.back() != L'\\' && normalized.back() != L'/') {
            normalized.push_back(L'\\');
        }
#else
        if (normalized.back() != L'/') {
            normalized.push_back(L'/');
        }
#endif
        return normalized;
    }

// -----------------------------------------------------------------------------
// Custom lightweight obfuscation routine
// -----------------------------------------------------------------------------
    std::wstring Encrypt(const std::wstring& input) {
        if (input.empty()) {
            return input;
        }

        std::wstring value = input;
        std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(std::towlower(c));
        });
        std::reverse(value.begin(), value.end());

        std::wstring transformed;
        for (size_t i = 0, offset = 0; i < value.size(); ++i, offset += 2) {
            int digit = static_cast<int>(value[i]) + static_cast<int>(offset);
            while (digit >= 10 || digit < 0) {
                digit = (digit / 10) + (digit % 10);
                digit = std::abs(digit) % 10;
            }
            transformed.push_back(static_cast<wchar_t>(L'0' + digit));
        }

        const size_t midpoint = (transformed.size() + 1) / 2;
        std::wstring left = transformed.substr(0, midpoint);
        std::wstring right = transformed.substr(midpoint);

        std::reverse(left.begin(), left.end());
        std::reverse(right.begin(), right.end());

        return right + left;
    }

// -----------------------------------------------------------------------------
// Retrieves the current user name as machine identifier
// -----------------------------------------------------------------------------
    std::wstring GetMachineId() {
#if defined(_WIN32)
        wchar_t buffer[256] = {};
        DWORD size = static_cast<DWORD>(std::size(buffer));
        return GetUserNameW(buffer, &size) ? buffer : L"UNKNOWN_USER";
#else
        const char* user = std::getenv("USER");
        if (!user) user = getlogin();
        if (user) {
            std::string s(user);
            return std::wstring(s.begin(), s.end());
        }
        return L"UNKNOWN_USER";
#endif
    }

// -----------------------------------------------------------------------------
// Window dragging simulation
// -----------------------------------------------------------------------------
    bool Drag(bijoy::core::NativeWindowHandle hwnd) {
#if defined(_WIN32)
        ReleaseCapture();
        SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return true;
#else
        (void)hwnd;
        return true;
#endif
    }

// -----------------------------------------------------------------------------
// Registry / Settings configuration
// -----------------------------------------------------------------------------
#if defined(_WIN32)
    namespace {
        constexpr const wchar_t* kRegistryPath = L"SOFTWARE\\Ekushe\\Settings";
        constexpr const wchar_t* kRegistryValueName = L"Password";
    }

    int GetRegistryPassword(std::wstring& out_password) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegistryPath, 0, KEY_READ, &key) != ERROR_SUCCESS) {
            return -1;
        }

        wchar_t buffer[256] = {};
        DWORD buffer_size = sizeof(buffer);
        DWORD value_type = 0;

        const LSTATUS status = RegQueryValueExW(
                key,
                kRegistryValueName,
                nullptr,
                &value_type,
                reinterpret_cast<LPBYTE>(buffer),
                &buffer_size);

        RegCloseKey(key);

        if (status != ERROR_SUCCESS || value_type != REG_SZ) {
            return -1;
        }

        out_password.assign(buffer);
        return 0;
    }

    int SetRegistryPassword(const std::wstring& password) {
        HKEY key = nullptr;
        if (RegCreateKeyExW(
                HKEY_CURRENT_USER,
                kRegistryPath,
                0,
                nullptr,
                0,
                KEY_WRITE,
                nullptr,
                &key,
                nullptr) != ERROR_SUCCESS) {
            return -1;
        }

        const DWORD data_size = static_cast<DWORD>((password.size() + 1) * sizeof(wchar_t));
        const LSTATUS status = RegSetValueExW(
                key,
                kRegistryValueName,
                0,
                REG_SZ,
                reinterpret_cast<const BYTE*>(password.c_str()),
                data_size);

        RegCloseKey(key);
        return status == ERROR_SUCCESS ? 0 : -1;
    }
#else
    namespace {
        std::wstring g_nonWinPassword;
    }

    int GetRegistryPassword(std::wstring& out_password) {
        out_password = g_nonWinPassword;
        return 0;
    }

    int SetRegistryPassword(const std::wstring& password) {
        g_nonWinPassword = password;
        return 0;
    }
#endif

    void InstallCrashHandler() {
        bijoy::error::InstallCrashDiagnostics();
    }
} // namespace bijoy::utils