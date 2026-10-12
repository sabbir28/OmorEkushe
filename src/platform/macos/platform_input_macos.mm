#if defined(__APPLE__)
#include "core/platform_input.h"
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#include <vector>

namespace bijoy::core {

void PlatformEmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend) {
    if (backspaces == 0 && toSend.empty()) {
        return;
    }

    // Emit Backspaces (macOS Virtual Keycode 51 is kVK_Delete / Backspace)
    for (size_t i = 0; i < backspaces; ++i) {
        CGEventRef down = CGEventCreateKeyboardEvent(nullptr, static_cast<CGKeyCode>(51), true);
        CGEventRef up = CGEventCreateKeyboardEvent(nullptr, static_cast<CGKeyCode>(51), false);
        if (down && up) {
            CGEventPost(kCGHIDEventTap, down);
            CGEventPost(kCGHIDEventTap, up);
            CFRelease(down);
            CFRelease(up);
        }
    }

    // Emit Unicode characters using CGEventKeyboardSetUnicodeString
    for (wchar_t c : toSend) {
        UniChar uni = static_cast<UniChar>(c);
        CGEventRef down = CGEventCreateKeyboardEvent(nullptr, 0, true);
        CGEventRef up = CGEventCreateKeyboardEvent(nullptr, 0, false);
        if (down && up) {
            CGEventKeyboardSetUnicodeString(down, 1, &uni);
            CGEventKeyboardSetUnicodeString(up, 1, &uni);
            CGEventPost(kCGHIDEventTap, down);
            CGEventPost(kCGHIDEventTap, up);
            CFRelease(down);
            CFRelease(up);
        }
    }
}

NativeWindowHandle PlatformGetActiveWindow() {
    NSRunningApplication* frontApp = [[NSWorkspace sharedWorkspace] frontmostApplication];
    if (frontApp) {
        return static_cast<NativeWindowHandle>([frontApp processIdentifier]);
    }
    return 0;
}

void PlatformSimulateKey(int keyCode, bool isKeyDown) {
    CGEventRef ev = CGEventCreateKeyboardEvent(nullptr, static_cast<CGKeyCode>(keyCode), isKeyDown);
    if (ev) {
        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
    }
}

} // namespace bijoy::core
#endif

