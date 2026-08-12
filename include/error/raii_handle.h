#pragma once

#include <utility>
#include <type_traits>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#endif

namespace bijoy::error {

// -----------------------------------------------------------------------------
// ScopeGuard: Executes a callable on scope exit (RAII cleanup barrier)
// Ensures cleanup code runs even if exceptions or early returns occur.
// -----------------------------------------------------------------------------
template <typename F>
class ScopeGuard {
public:
    explicit ScopeGuard(F&& fn) : m_fn(std::forward<F>(fn)), m_active(true) {}
    ~ScopeGuard() {
        if (m_active) {
            m_fn();
        }
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ScopeGuard(ScopeGuard&& other) noexcept
        : m_fn(std::move(other.m_fn)), m_active(other.m_active) {
        other.m_active = false;
    }

    ScopeGuard& operator=(ScopeGuard&& other) noexcept {
        if (this != &other) {
            if (m_active) {
                m_fn();
            }
            m_fn = std::move(other.m_fn);
            m_active = other.m_active;
            other.m_active = false;
        }
        return *this;
    }

    void Dismiss() noexcept { m_active = false; }

private:
    F m_fn;
    bool m_active;
};

template <typename F>
ScopeGuard<std::decay_t<F>> MakeScopeGuard(F&& fn) {
    return ScopeGuard<std::decay_t<F>>(std::forward<F>(fn));
}

// -----------------------------------------------------------------------------
// UniqueHandle: Exception-safe RAII wrapper for generic Win32 / OS handles
// -----------------------------------------------------------------------------
template <typename HandleType, typename Traits>
class UniqueHandle {
public:
    UniqueHandle() noexcept : m_handle(Traits::InvalidValue()) {}
    explicit UniqueHandle(HandleType handle) noexcept : m_handle(handle) {}

    ~UniqueHandle() noexcept {
        Reset();
    }

    UniqueHandle(const UniqueHandle&) = delete;
    UniqueHandle& operator=(const UniqueHandle&) = delete;

    UniqueHandle(UniqueHandle&& other) noexcept
        : m_handle(other.m_handle) {
        other.m_handle = Traits::InvalidValue();
    }

    UniqueHandle& operator=(UniqueHandle&& other) noexcept {
        if (this != &other) {
            Reset(other.m_handle);
            other.m_handle = Traits::InvalidValue();
        }
        return *this;
    }

    [[nodiscard]] HandleType Get() const noexcept { return m_handle; }
    [[nodiscard]] bool IsValid() const noexcept { return m_handle != Traits::InvalidValue(); }
    explicit operator bool() const noexcept { return IsValid(); }

    HandleType* Put() noexcept {
        Reset();
        return &m_handle;
    }

    HandleType Release() noexcept {
        HandleType temp = m_handle;
        m_handle = Traits::InvalidValue();
        return temp;
    }

    void Reset(HandleType handle = Traits::InvalidValue()) noexcept {
        if (m_handle != Traits::InvalidValue()) {
            Traits::Close(m_handle);
        }
        m_handle = handle;
    }

private:
    HandleType m_handle;
};

#if defined(_WIN32)

struct Win32HookTraits {
    static HHOOK InvalidValue() noexcept { return nullptr; }
    static void Close(HHOOK hook) noexcept { UnhookWindowsHookEx(hook); }
};
using UniqueHook = UniqueHandle<HHOOK, Win32HookTraits>;

struct Win32KeyTraits {
    static HKEY InvalidValue() noexcept { return nullptr; }
    static void Close(HKEY key) noexcept { RegCloseKey(key); }
};
using UniqueHKey = UniqueHandle<HKEY, Win32KeyTraits>;

struct Win32WindowTraits {
    static HWND InvalidValue() noexcept { return nullptr; }
    static void Close(HWND hwnd) noexcept {
        if (IsWindow(hwnd)) {
            DestroyWindow(hwnd);
        }
    }
};
using UniqueHWND = UniqueHandle<HWND, Win32WindowTraits>;

struct Win32ModuleTraits {
    static HMODULE InvalidValue() noexcept { return nullptr; }
    static void Close(HMODULE hmod) noexcept { FreeLibrary(hmod); }
};
using UniqueHModule = UniqueHandle<HMODULE, Win32ModuleTraits>;

#endif

} // namespace bijoy::error
