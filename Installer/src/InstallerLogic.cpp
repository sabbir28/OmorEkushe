#include "InstallerLogic.h"
#include "Resources.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;

namespace InstallerLogic {

    std::atomic<bool> g_is_installing{false};
    Progress g_current_progress{0.0f, L"", false, ""};

    bool CreateShortcut(const std::wstring& targetPath, const std::wstring& shortcutPath, const std::wstring& description) {
        HRESULT hres;
        IShellLink* psl;

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (SUCCEEDED(hres)) {
            psl->SetPath(targetPath.c_str());
            psl->SetDescription(description.c_str());
            psl->SetWorkingDirectory(fs::path(targetPath).parent_path().c_str());

            IPersistFile* ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
            if (SUCCEEDED(hres)) {
                hres = ppf->Save(shortcutPath.c_str(), TRUE);
                ppf->Release();
            }
            psl->Release();
        }
        return SUCCEEDED(hres);
    }

    bool SetStartup(const std::wstring& appName, const std::wstring& appPath, bool enable) {
        HKEY hKey;
        const wchar_t* runKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        if (RegOpenKeyExW(HKEY_CURRENT_USER, runKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            if (enable) {
                RegSetValueExW(hKey, appName.c_str(), 0, REG_SZ, (BYTE*)appPath.c_str(), (DWORD)((appPath.length() + 1) * sizeof(wchar_t)));
            } else {
                RegDeleteValueW(hKey, appName.c_str());
            }
            RegCloseKey(hKey);
            return true;
        }
        return false;
    }

    void InstallThread(Options options) {
        try {
            fs::path installDir = options.installPath;
            if (!fs::exists(installDir)) {
                fs::create_directories(installDir);
            }

            for (size_t i = 0; i < g_embedded_files_count; ++i) {
                const auto& file = g_embedded_files[i];
                fs::path targetPath = installDir / file.path;
                
                g_current_progress.currentFile = targetPath.wstring();
                g_current_progress.percentage = (float)i / (float)g_embedded_files_count;

                fs::create_directories(targetPath.parent_path());
                std::ofstream out(targetPath, std::ios::binary);
                if (!out) {
                    g_current_progress.errorMessage = "Failed to create file: " + targetPath.string();
                    g_current_progress.finished = true;
                    g_is_installing = false;
                    return;
                }
                out.write((const char*)file.data, file.size);
                out.close();
            }

            // Create Desktop Shortcut
            if (options.createDesktopShortcut) {
                wchar_t desktopPath[MAX_PATH];
                SHGetSpecialFolderPathW(NULL, desktopPath, CSIDL_DESKTOPDIRECTORY, FALSE);
                fs::path shortcutPath = fs::path(desktopPath) / L"OmorEkushe.lnk";
                fs::path exePath = installDir / "bin" / "OmorEkushe.exe";
                CreateShortcut(exePath.wstring(), shortcutPath.wstring(), L"OmorEkushe Application");
            }

            // Set Startup
            if (options.runOnStartup) {
                fs::path exePath = installDir / "bin" / "OmorEkushe.exe";
                SetStartup(L"OmorEkushe", exePath.wstring(), true);
            }

            g_current_progress.percentage = 1.0f;
            g_current_progress.currentFile = L"Installation Complete";
            g_current_progress.finished = true;
        } catch (const std::exception& e) {
            g_current_progress.errorMessage = e.what();
            g_current_progress.finished = true;
        }
        g_is_installing = false;
    }

    bool StartInstallation(const Options& options, Progress& progress) {
        if (g_is_installing) return false;
        g_is_installing = true;
        g_current_progress = {0.0f, L"", false, ""};
        std::thread(InstallThread, options).detach();
        return true;
    }

    void UpdateInstallation() {
        // Nothing to do here for now as history is updated in thread
    }
}
