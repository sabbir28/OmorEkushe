#include "core/layout_discovery.h"
#include "utils/system_utils.h"

#include <shlwapi.h>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

namespace bijoy::core {

    std::wstring GetAppDirectory() {
        wchar_t path[MAX_PATH] = {};
        if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) {
            return L"";
        }
        PathRemoveFileSpecW(path);
        return bijoy::utils::EnsureTrailingBackslash(path);
    }

    namespace {

        void FindXmlRecursive(const std::wstring& dir, std::vector<std::wstring>& out) {
            const std::wstring search = dir + L"*";
            WIN32_FIND_DATAW findData = {};
            HANDLE handle = FindFirstFileW(search.c_str(), &findData);
            if (handle == INVALID_HANDLE_VALUE) {
                return;
            }

            do {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
                    if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                        FindXmlRecursive(dir + findData.cFileName + L"\\", out);
                    }
                } else if (wcsstr(findData.cFileName, L".xml") != nullptr) {
                    out.push_back(dir + findData.cFileName);
                }
            } while (FindNextFileW(handle, &findData));

            FindClose(handle);
        }

    } // namespace

    bool FindLayouts(std::vector<Layout>& layouts, const std::wstring& appDir) {
        const std::wstring layoutDirs[] = {
            appDir + L"Layouts\\",
            appDir + L"..\\data\\layout\\",
            appDir + L"..\\data\\Layouts\\"
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
                layouts.push_back(std::move(layout));
            }
        }
        return !layouts.empty();
    }

} // namespace bijoy::core
