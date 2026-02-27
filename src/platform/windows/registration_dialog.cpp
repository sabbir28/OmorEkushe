#include "platform/windows/registration_dialog.h"

#include "platform/windows/resource.h"
#include "utils/system_utils.h"

#include <commctrl.h>
#include <string>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

namespace bijoy::platform::windows {

    namespace {

// -----------------------------------------------------------------------------
// Reads text content from a dialog control by ID
// Returns empty string if control is missing or empty
// -----------------------------------------------------------------------------
        std::wstring GetDialogItemText(HWND dialog, int id) {
            HWND control = GetDlgItem(dialog, id);
            if (!control) {
                return L"";
            }

            const int length = GetWindowTextLengthW(control) + 1;
            if (length <= 1) {
                return L"";
            }

            std::wstring value(length, L'\0');
            GetWindowTextW(control, value.data(), length);
            value.resize(length - 1);

            return value;
        }

// -----------------------------------------------------------------------------
// Registration dialog procedure
// Responsibility:
// - Validate registration state
// - Return success/failure
// -----------------------------------------------------------------------------
        INT_PTR CALLBACK RegistrationDialogProc(
                HWND dialog,
                UINT msg,
                WPARAM wParam,
                LPARAM lParam) {

            (void)lParam;

            switch (msg) {

                case WM_INITDIALOG: {
                    std::wstring savedPassword;
                    bijoy::utils::GetRegistryPassword(savedPassword);

                    // Fast-path: already registered
                    if (savedPassword == bijoy::utils::Password) {
                        EndDialog(dialog, 1);
                        return TRUE;
                    }

                    return TRUE;
                }

                case WM_CLOSE:
                    EndDialog(dialog, 0);
                    return TRUE;

                case WM_COMMAND:

                    // Register / OK
                    if (LOWORD(wParam) == IDOK_BTN) {

                        const std::wstring enteredPassword =
                                GetDialogItemText(dialog, IDC_PASSWORD_EDIT) +
                                GetDialogItemText(dialog, IDC_PASSWORD1) +
                                GetDialogItemText(dialog, IDC_PASSWORD2) +
                                GetDialogItemText(dialog, IDC_PASSWORD3) +
                                GetDialogItemText(dialog, IDC_PASSWORD4);

                        if (!enteredPassword.empty() &&
                            enteredPassword == bijoy::utils::Password) {

                            bijoy::utils::SetRegistryPassword(
                                    bijoy::utils::Password);

                            EndDialog(dialog, 1);
                        } else {
                            MessageBoxW(
                                    dialog,
                                    L"Wrong Password",
                                    L"Omor Ekushe",
                                    MB_OK | MB_ICONERROR);
                        }
                        return TRUE;
                    }

                    // Cancel
                    if (LOWORD(wParam) == IDCANCEL_BTN) {
                        EndDialog(dialog, 0);
                        return TRUE;
                    }
                    break;
            }

            return FALSE;
        }

    } // namespace

// -----------------------------------------------------------------------------
// Shows registration dialog
// Return value:
//   1 -> registration successful
//   0 -> cancelled or failed
// -----------------------------------------------------------------------------
    int ShowRegistrationDialog(HWND parent) {
        return static_cast<int>(
                DialogBoxW(
                        GetModuleHandle(nullptr),
                        MAKEINTRESOURCEW(IDD_REGISTRATION),
                        parent,
                        RegistrationDialogProc));
    }

} // namespace bijoy::platform::windows