#pragma once

#include <string>
#include "core/platform_types.h"

namespace bijoy::utils {

extern std::wstring Password;

std::wstring EnsureTrailingBackslash(const std::wstring& path);
std::wstring Encrypt(const std::wstring& number);
std::wstring GetMachineId();
bool Drag(bijoy::core::NativeWindowHandle hwnd);
int GetRegistryPassword(std::wstring& out);
int SetRegistryPassword(const std::wstring& value);
void InstallCrashHandler();

} // namespace bijoy::utils

