#pragma once

#include <string>
#if defined(_WIN32)
#include <windows.h>
#endif

namespace bijoy::error {

struct CrashInfo {
    unsigned long exceptionCode{0};
    void* exceptionAddress{nullptr};
    std::wstring exceptionName;
    std::wstring moduleName;
    std::wstring crashLogPath;
    std::wstring miniDumpPath;
};

// -----------------------------------------------------------------------------
// Installs ultra-level crash diagnostic handlers:
// 1. Windows Vectored Exception Handler (VEH)
// 2. Windows Unhandled Exception Filter
// 3. C++ CRT terminate and unexpected handlers
// 4. CRT invalid parameter & purecall handlers
// 5. POSIX C signal traps (SIGSEGV, SIGFPE, SIGILL, SIGABRT)
// -----------------------------------------------------------------------------
void InstallCrashDiagnostics();

// Forcefully generates a minidump and crash report programmatically
bool GenerateCrashReport(void* exceptionPointers = nullptr, const wchar_t* customReason = nullptr);

// Formats human-readable exception code description
std::wstring GetExceptionString(unsigned long code);

} // namespace bijoy::error
