#pragma once
#include <string>

namespace editor {

std::string WStringToUtf8(const std::wstring& wstr);
std::wstring Utf8ToWString(const std::string& str);
std::wstring GetExeDirectory();

} // namespace editor
