// =============================================================================
// File   : layout_discovery.cpp
// Module : core (Layout Auto-Discovery)
// Purpose: Discovers and loads keyboard layout XML files at application startup
//          across Windows, Linux, and macOS using C++17 std::filesystem.
// =============================================================================

#include "core/layout_discovery.h"
#include "core/startup_options.h"
#include "core/keyboard_hook_service.h"
#include "utils/system_utils.h"

#include <filesystem>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace bijoy::core {

#ifndef VK_F12
    constexpr int VK_F12 = 0x7B;
#endif

    std::wstring GetAppDirectory() {
#if defined(_WIN32)
        wchar_t path[MAX_PATH] = {};
        if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) {
            return L"";
        }
        PathRemoveFileSpecW(path);
        return bijoy::utils::EnsureTrailingBackslash(path);
#elif defined(__APPLE__)
        char path[PATH_MAX] = {};
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            std::filesystem::path p(path);
            return p.parent_path().wstring() + L"/";
        }
        return L"";
#else
        char path[PATH_MAX] = {};
        ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (count > 0) {
            path[count] = '\0';
            std::filesystem::path p(path);
            return p.parent_path().wstring() + L"/";
        }
        return L"";
#endif
    }

    namespace {

        void FindXmlRecursive(const std::wstring& dirPath, std::vector<std::wstring>& out) {
            std::error_code ec;
            std::filesystem::path dir(dirPath);
            if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
                return;
            }

            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, std::filesystem::directory_options::skip_permission_denied, ec)) {
                if (ec) break;
                if (entry.is_regular_file(ec) && entry.path().extension() == ".xml") {
                    out.push_back(entry.path().wstring());
                }
            }
        }

    } // namespace

    bool FindLayouts(std::vector<Layout>& layouts, const std::wstring& appDir) {
        const std::wstring slash = L"/";
        const std::wstring layoutDirs[] = {
                appDir + L"Layouts/",
                appDir + L"data/Layouts/",
                appDir + L"data/layout/",
                appDir + L"../data/layout/",
                appDir + L"../data/Layouts/",
                appDir + L"Layouts\\",
                appDir + L"data\\Layouts\\",
                appDir + L"..\\data\\Layouts\\",
#ifdef BIJOY_DATA_DIR
                std::wstring(BIJOY_DATA_DIR) + L"Layouts/",
                std::wstring(BIJOY_DATA_DIR) + L"Layouts\\",
#endif
        };

        std::vector<std::wstring> files;
        for (const auto& layoutDir : layoutDirs) {
            FindXmlRecursive(layoutDir, files);
            if (!files.empty()) {
                break;
            }
        }

        if (files.empty()) {
            return false;
        }

        layouts.clear();
        for (size_t i = 0; i < files.size(); ++i) {
            Layout layout;
            if (layout.loadFromFile(files[i].c_str())) {
                layout.id = static_cast<int>(i);

                LayoutShortcut customShortcut;
                if (LoadCustomLayoutShortcut(layout.name, customShortcut)) {
                    layout.shortcut = customShortcut;
                }

                layouts.push_back(std::move(layout));
            }
        }

        bool hasPhonetic = false;
        for (auto& l : layouts) {
            if (l.isPhonetic) {
                hasPhonetic = true;
                int key = VK_F12;
                bool ctrl = false, alt = false, shift = false;
                GetPhoneticToggleShortcut(key, ctrl, alt, shift);
                l.shortcut.keyCode = key;
                l.shortcut.ctrl = ctrl;
                l.shortcut.alt = alt;
                l.shortcut.shift = shift;
                break;
            }
        }
        if (!hasPhonetic) {
            Layout phoneticLayout;
            phoneticLayout.name = L"Phonetic";
            phoneticLayout.iconName = L"Phonetic";
            phoneticLayout.isPhonetic = true;
            int key = VK_F12;
            bool ctrl = false, alt = false, shift = false;
            GetPhoneticToggleShortcut(key, ctrl, alt, shift);
            phoneticLayout.shortcut.keyCode = key;
            phoneticLayout.shortcut.ctrl = ctrl;
            phoneticLayout.shortcut.alt = alt;
            phoneticLayout.shortcut.shift = shift;
            phoneticLayout.id = static_cast<int>(layouts.size());
            layouts.push_back(std::move(phoneticLayout));
        }

        return !layouts.empty();
    }

} // namespace bijoy::core

