// =============================================================================
// File   : crash_handler.cpp
// Module : error (Crash Diagnostic Shield)
// Purpose: Implements a multi-layered crash safety net for Omor Ekushe.
//          The goal is to prevent silent termination and always produce
//          human-readable diagnostic output before any process death.
//
//          Protection layers (installed in order by InstallCrashDiagnostics):
//            1. Vectored Exception Handler (VEH) — catches hardware faults
//               (access violations, stack overflow, illegal instructions)
//               BEFORE the OS unhandled exception filter.
//            2. SetUnhandledExceptionFilter — catches any remaining SEH
//               exceptions not handled by Layer 1.
//            3. CRT error handlers — catches invalid parameter calls and
//               pure virtual function calls inside the C Runtime.
//            4. std::set_terminate — catches unhandled C++ exceptions that
//               trigger std::terminate().
//            5. POSIX signal handlers — catches SIGSEGV, SIGABRT, SIGFPE,
//               SIGILL from C-level signal delivery.
//
//          On any fatal event, ProcessCrash() performs:
//            a) Atomic re-entrancy guard (prevents recursive crash handler)
//            b) MiniDump generation via dbghelp.dll (crash.dmp)
//            c) Text crash log generation (crash.log)
//            d) User-facing MessageBox with diagnostic details
//            e) Process termination via ExitProcess(1)
// =============================================================================

#include "error/crash_handler.h"

#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#include <shlobj.h>

#ifndef MiniDumpWithNormalMemory
#define MiniDumpWithNormalMemory (MINIDUMP_TYPE)0
#endif
#endif

namespace bijoy::error {

namespace {

// Lock to prevent recursive execution of crash handler
std::atomic<bool> g_isHandlingCrash{false};

typedef BOOL(WINAPI* MiniDumpWriteDumpFn)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

// Helper to resolve exception code names
const wchar_t* ExceptionCodeToText(DWORD code) {
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: return L"EXCEPTION_ACCESS_VIOLATION (0xC0000005)";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED (0xC000008C)";
        case EXCEPTION_BREAKPOINT: return L"EXCEPTION_BREAKPOINT (0x80000003)";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return L"EXCEPTION_DATATYPE_MISALIGNMENT (0x80000002)";
        case EXCEPTION_FLT_DENORMAL_OPERAND: return L"EXCEPTION_FLT_DENORMAL_OPERAND (0xC000008D)";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return L"EXCEPTION_FLT_DIVIDE_BY_ZERO (0xC000008E)";
        case EXCEPTION_FLT_INEXACT_RESULT: return L"EXCEPTION_FLT_INEXACT_RESULT (0xC000008F)";
        case EXCEPTION_FLT_INVALID_OPERATION: return L"EXCEPTION_FLT_INVALID_OPERATION (0xC0000090)";
        case EXCEPTION_FLT_OVERFLOW: return L"EXCEPTION_FLT_OVERFLOW (0xC0000091)";
        case EXCEPTION_FLT_STACK_CHECK: return L"EXCEPTION_FLT_STACK_CHECK (0xC0000092)";
        case EXCEPTION_FLT_UNDERFLOW: return L"EXCEPTION_FLT_UNDERFLOW (0xC0000093)";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return L"EXCEPTION_ILLEGAL_INSTRUCTION (0xC000001D)";
        case EXCEPTION_IN_PAGE_ERROR: return L"EXCEPTION_IN_PAGE_ERROR (0xC0000006)";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return L"EXCEPTION_INT_DIVIDE_BY_ZERO (0xC0000094)";
        case EXCEPTION_INT_OVERFLOW: return L"EXCEPTION_INT_OVERFLOW (0xC0000095)";
        case EXCEPTION_INVALID_DISPOSITION: return L"EXCEPTION_INVALID_DISPOSITION (0xC0000026)";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"EXCEPTION_NONCONTINUABLE_EXCEPTION (0xC0000025)";
        case EXCEPTION_PRIV_INSTRUCTION: return L"EXCEPTION_PRIV_INSTRUCTION (0xC0000096)";
        case EXCEPTION_SINGLE_STEP: return L"EXCEPTION_SINGLE_STEP (0x80000004)";
        case EXCEPTION_STACK_OVERFLOW: return L"EXCEPTION_STACK_OVERFLOW (0xC00000FD)";
        default: return L"UNKNOWN_EXCEPTION";
    }
}

std::wstring GetModulePathForAddress(void* addr) {
#if defined(_WIN32)
    HMODULE hMod = nullptr;
    if (GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            static_cast<LPCWSTR>(addr),
            &hMod) && hMod) {
        wchar_t path[MAX_PATH] = {};
        if (GetModuleFileNameW(hMod, path, MAX_PATH)) {
            return path;
        }
    }
#endif
    return L"Unknown Module";
}

void WriteCrashLog(const CrashInfo& info) {
#if defined(_WIN32)
    FILE* f = nullptr;
    if (_wfopen_s(&f, info.crashLogPath.c_str(), L"w") == 0 && f) {
        SYSTEMTIME st;
        GetLocalTime(&st);

        fwprintf(f, L"=====================================================\n");
        fwprintf(f, L"        OMOR EKUSHE FATAL CRASH REPORT               \n");
        fwprintf(f, L"=====================================================\n");
        fwprintf(f, L"Timestamp     : %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        fwprintf(f, L"Exception Code: 0x%08X (%s)\n", info.exceptionCode, info.exceptionName.c_str());
        fwprintf(f, L"Fault Address : 0x%p\n", info.exceptionAddress);
        fwprintf(f, L"Fault Module  : %s\n", info.moduleName.c_str());
        fwprintf(f, L"Process ID    : %dw\n", GetCurrentProcessId());
        fwprintf(f, L"Thread ID     : %dw\n", GetCurrentThreadId());
        if (!info.miniDumpPath.empty()) {
            fwprintf(f, L"MiniDump Path : %s\n", info.miniDumpPath.c_str());
        }
        fwprintf(f, L"=====================================================\n");
        fclose(f);
    }
#endif
}

bool WriteMiniDump(EXCEPTION_POINTERS* pExcPtrs, const std::wstring& dumpPath) {
#if defined(_WIN32)
    HMODULE hDbgHelp = LoadLibraryW(L"dbghelp.dll");
    if (!hDbgHelp) {
        return false;
    }

    auto pfnMiniDumpWriteDump = reinterpret_cast<MiniDumpWriteDumpFn>(
        GetProcAddress(hDbgHelp, "MiniDumpWriteDump"));
    if (!pfnMiniDumpWriteDump) {
        FreeLibrary(hDbgHelp);
        return false;
    }

    HANDLE hFile = CreateFileW(
        dumpPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (hFile == INVALID_HANDLE_VALUE) {
        FreeLibrary(hDbgHelp);
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = pExcPtrs;
    mei.ClientPointers = FALSE;

    BOOL result = pfnMiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpWithNormalMemory,
        pExcPtrs ? &mei : nullptr,
        nullptr,
        nullptr);

    CloseHandle(hFile);
    FreeLibrary(hDbgHelp);
    return result == TRUE;
#else
    return false;
#endif
}

void ProcessCrash(EXCEPTION_POINTERS* pExceptionInfo, const wchar_t* customReason = nullptr) {
    if (g_isHandlingCrash.exchange(true)) {
        // Prevent infinite recursive crashes inside crash handler
        ExitProcess(1);
        return;
    }

    CrashInfo info;
    info.exceptionCode = pExceptionInfo ? pExceptionInfo->ExceptionRecord->ExceptionCode : 0xE0000001;
    info.exceptionAddress = pExceptionInfo ? pExceptionInfo->ExceptionRecord->ExceptionAddress : nullptr;
    info.exceptionName = customReason ? customReason : ExceptionCodeToText(info.exceptionCode);
    info.moduleName = GetModulePathForAddress(info.exceptionAddress);

    wchar_t appPath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, appPath, MAX_PATH);
    wchar_t* lastSlash = wcsrchr(appPath, L'\\');
    if (lastSlash) *lastSlash = L'\0';

    info.crashLogPath = std::wstring(appPath) + L"\\crash.log";
    info.miniDumpPath = std::wstring(appPath) + L"\\crash.dmp";

    // Create minidump
    WriteMiniDump(pExceptionInfo, info.miniDumpPath);

    // Create log file
    WriteCrashLog(info);

    // Display rich user alert
    wchar_t alertMsg[1024] = {};
    swprintf_s(alertMsg,
        L"Omor Ekushe encountered an unexpected fatal error and has been safely stopped.\n\n"
        L"Reason / Code: %s\n"
        L"Fault Address : 0x%p\n"
        L"Fault Module  : %s\n\n"
        L"Diagnostic files have been written:\n"
        L"Log : %s\n"
        L"Dump: %s",
        info.exceptionName.c_str(),
        info.exceptionAddress,
        info.moduleName.c_str(),
        info.crashLogPath.c_str(),
        info.miniDumpPath.c_str());

    MessageBoxW(
        nullptr,
        alertMsg,
        L"Omor Ekushe - Crash Prevention & Diagnostic Shield",
        MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);

    ExitProcess(1);
}

LONG WINAPI CustomVectoredExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo) {
    if (pExceptionInfo && pExceptionInfo->ExceptionRecord) {
        DWORD code = pExceptionInfo->ExceptionRecord->ExceptionCode;
        // Ignore non-fatal debug / status events
        if (code == DBG_PRINTEXCEPTION_C || code == 0x40010006 || code == STATUS_BREAKPOINT) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        // Intercept severe hardware/memory violations early
        if (code == EXCEPTION_ACCESS_VIOLATION ||
            code == EXCEPTION_STACK_OVERFLOW ||
            code == EXCEPTION_ILLEGAL_INSTRUCTION ||
            code == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ||
            code == EXCEPTION_DATATYPE_MISALIGNMENT) {
            ProcessCrash(pExceptionInfo);
            return EXCEPTION_EXECUTE_HANDLER;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI CustomUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo) {
    ProcessCrash(pExceptionInfo);
    return EXCEPTION_EXECUTE_HANDLER;
}

void CustomInvalidParameterHandler(
    const wchar_t* expression,
    const wchar_t* function,
    const wchar_t* file,
    unsigned int line,
    uintptr_t pReserved) {
    (void)expression; (void)function; (void)file; (void)line; (void)pReserved;
    ProcessCrash(nullptr, L"Invalid CRT Parameter Exception");
}

void CustomPureCallHandler() {
    ProcessCrash(nullptr, L"Pure Virtual Function Call Exception");
}

void CustomSignalHandler(int signal) {
    const wchar_t* sigName = L"C Signal Error";
    switch (signal) {
        case SIGSEGV: sigName = L"SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: sigName = L"SIGABRT (Abort)"; break;
        case SIGFPE:  sigName = L"SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  sigName = L"SIGILL (Illegal Instruction)"; break;
    }
    ProcessCrash(nullptr, sigName);
}

void CustomTerminateHandler() {
    ProcessCrash(nullptr, L"std::terminate Called (Unhandled C++ Exception)");
}

} // namespace

void InstallCrashDiagnostics() {
#if defined(_WIN32)
    // 1. Vectored Exception Handler (VEH)
    AddVectoredExceptionHandler(1, CustomVectoredExceptionHandler);

    // 2. Unhandled Exception Filter
    SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

    // 3. CRT Error Handlers
    _set_invalid_parameter_handler(CustomInvalidParameterHandler);
    _set_purecall_handler(CustomPureCallHandler);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    // 4. C++ Runtime Termination Handler
    std::set_terminate(CustomTerminateHandler);

    // 5. POSIX / C Signal Handlers
    std::signal(SIGSEGV, CustomSignalHandler);
    std::signal(SIGABRT, CustomSignalHandler);
    std::signal(SIGFPE,  CustomSignalHandler);
    std::signal(SIGILL,  CustomSignalHandler);
}

bool GenerateCrashReport(void* exceptionPointers, const wchar_t* customReason) {
    ProcessCrash(static_cast<EXCEPTION_POINTERS*>(exceptionPointers), customReason);
    return true;
}

std::wstring GetExceptionString(unsigned long code) {
    return ExceptionCodeToText(code);
}

} // namespace bijoy::error
