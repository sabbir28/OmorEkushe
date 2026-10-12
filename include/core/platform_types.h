#pragma once

#include <cstdint>
#include <string>

#if defined(linux)
#undef linux
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace bijoy::core {
    using NativeWindowHandle = HWND;
    constexpr NativeWindowHandle kInvalidWindowHandle = nullptr;
    constexpr ULONG_PTR OMOR_EKUSHE_EXTRA_INFO = 0x0E1952;
}
#else
namespace bijoy::core {
    using NativeWindowHandle = uintptr_t;
    constexpr NativeWindowHandle kInvalidWindowHandle = 0;
}
#endif

