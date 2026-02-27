#pragma once

#include <string>
#include <windows.h>

namespace bijoy::utils {

extern std::wstring Password;

std::wstring EnsureTrailingBackslash(const std::wstring& path);
std::wstring Encrypt(const std::wstring& number);
std::wstring GetMachineId();
bool Drag(HWND hwnd);
int GetRegistryPassword(std::wstring& out);
int SetRegistryPassword(const std::wstring& value);

} // namespace bijoy::utils
