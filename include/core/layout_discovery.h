#pragma once

#include "core/layout.h"
#include <string>
#include <vector>

namespace bijoy::core {

bool FindLayouts(std::vector<Layout>& layouts, const std::wstring& appDir);
std::wstring GetAppDirectory();

} // namespace bijoy::core
