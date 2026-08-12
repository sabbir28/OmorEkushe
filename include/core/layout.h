#pragma once

#include <map>
#include <string>

namespace bijoy::core {

struct LayoutShortcut {
  bool alt = false;
  bool ctrl = false;
  bool shift = false;
  int keyCode = 0;
};

struct KeyMapping {
  std::wstring normal;
  std::wstring shift;
  std::wstring normalOption;
  std::wstring shiftOption;
};

struct Layout {
  std::wstring name;
  std::wstring iconName;
  std::wstring path;
  int id = 0;
  LayoutShortcut shortcut;
  std::map<int, KeyMapping> key;
  std::map<std::wstring, std::wstring> juk;

  void clear();
  bool loadFromFile(const wchar_t* filePath);
  bool updateFromFile();
};

} // namespace bijoy::core
