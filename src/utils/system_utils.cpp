#include "utils/system_utils.h"

#include <algorithm>
#include <windows.h>

namespace bijoy::utils {

// -----------------------------------------------------------------------------
// Global static password seed
// Used as a base constant for encryption-related logic
// -----------------------------------------------------------------------------
// Key : RN28 - T29S - K1XM - J6XY - LK24
    std::wstring Password = L"RN28T29SK1XMJ6XYLK24";

// -----------------------------------------------------------------------------
// Ensures a filesystem path always ends with a backslash
// Prevents path concatenation bugs downstream
// -----------------------------------------------------------------------------
    std::wstring EnsureTrailingBackslash(const std::wstring& path) {
        if (path.empty()) {
            return path;
        }

        std::wstring normalized = path;
        if (normalized.back() != L'\\') {
            normalized.push_back(L'\\');
        }
        return normalized;
    }

// -----------------------------------------------------------------------------
// Custom lightweight obfuscation routine
// 1. Normalize input to lowercase
// 2. Reverse the string
// 3. Apply position-based numeric scrambling
// 4. Split, reverse halves, and recombine
//
// NOTE: This is NOT cryptographically secure.
// Intended only for basic deterrence.
// -----------------------------------------------------------------------------
    std::wstring Encrypt(const std::wstring& input) {
        std::wstring value = input;

        // Normalize input
        std::transform(value.begin(), value.end(), value.begin(), ::towlower);

        // Reverse string
        std::reverse(value.begin(), value.end());

        // Position-based digit transformation
        std::wstring transformed;
        for (size_t i = 0, offset = 0; i < value.size(); ++i, offset += 2) {
            int digit = static_cast<int>(value[i]) + static_cast<int>(offset);

            // Reduce to single digit
            while (digit >= 10 || digit < 0) {
                digit = (digit / 10) + (digit % 10);
                digit = std::abs(digit) % 10;
            }

            transformed.push_back(static_cast<wchar_t>(L'0' + digit));
        }

        // Split and reverse halves for added diffusion
        const size_t midpoint = (transformed.size() + 1) / 2;
        std::wstring left = transformed.substr(0, midpoint);
        std::wstring right = transformed.substr(midpoint);

        std::reverse(left.begin(), left.end());
        std::reverse(right.begin(), right.end());

        return right + left;
    }

// -----------------------------------------------------------------------------
// Retrieves the local machine name
// Used as a lightweight machine identifier
// -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
// Retrieves the current Windows user name
// Lightweight, fast, no registry access
// -----------------------------------------------------------------------------
    std::wstring GetMachineId() {
        wchar_t buffer[256] = {};
        DWORD size = static_cast<DWORD>(std::size(buffer));

        return GetUserNameW(buffer, &size)
               ? buffer
               : L"UNKNOWN_USER";
    }

// -----------------------------------------------------------------------------
// Enables window dragging by simulating a title-bar mouse event
// Typically used for borderless windows
// -----------------------------------------------------------------------------
    bool Drag(HWND hwnd) {
        ReleaseCapture();
        SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return true;
    }

// -----------------------------------------------------------------------------
// Registry configuration
// Separate vendor namespace for clean OS hygiene
// -----------------------------------------------------------------------------
    namespace {

        constexpr const wchar_t* kRegistryPath =
                L"SOFTWARE\\Ekushe\\Settings";

        constexpr const wchar_t* kRegistryValueName =
                L"Password";

    }  // namespace

// -----------------------------------------------------------------------------
// Reads the stored password value from registry
// Returns 0 on success, -1 on failure
// -----------------------------------------------------------------------------
    int GetRegistryPassword(std::wstring& out_password) {
        HKEY key = nullptr;

        if (RegOpenKeyExW(
                HKEY_CURRENT_USER,
                kRegistryPath,
                0,
                KEY_READ,
                &key) != ERROR_SUCCESS) {
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

// -----------------------------------------------------------------------------
// Writes the password value into registry
// Creates the key path if it does not exist
// -----------------------------------------------------------------------------
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

        const DWORD data_size =
                static_cast<DWORD>((password.size() + 1) * sizeof(wchar_t));

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

}  // namespace bijoy::utils