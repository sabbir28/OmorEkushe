#pragma once
#include <string>
#include <vector>
#include <atomic>
#include <system_error>

namespace InstallerLogic {
    struct Options {
        std::wstring installPath;
        bool createDesktopShortcut;
        bool runOnStartup;
    };

    struct Progress {
        float percentage;
        std::wstring currentFile;
        bool finished;
        std::string errorMessage;
    };

    bool StartInstallation(const Options& options, Progress& progress);
    void UpdateInstallation();
    
    extern std::atomic<bool> g_is_installing;
    extern Progress g_current_progress;
    
    // Windows helpers
    bool CreateShortcut(const std::wstring& targetPath, const std::wstring& shortcutPath, const std::wstring& description);
    bool SetStartup(const std::wstring& appName, const std::wstring& appPath, bool enable);
}
