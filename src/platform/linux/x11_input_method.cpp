#if !defined(_WIN32) && !defined(__APPLE__)
#include "platform/linux/x11_input_method.h"
#include <iostream>
#include <vector>
#include <codecvt>
#include <locale>
#include <cstring>

#if __has_include(<X11/Xlib.h>)
#include <X11/Xlib.h>
#include <X11/keysym.h>
#if __has_include(<X11/extensions/XTest.h>)
#include <X11/extensions/XTest.h>
#define HAVE_XTEST 1
#endif
#define HAVE_XLIB 1
#endif

namespace bijoy::platform::linux {

X11InputEngine& X11InputEngine::Instance() {
    static X11InputEngine instance;
    return instance;
}

X11InputEngine::X11InputEngine() = default;

X11InputEngine::~X11InputEngine() {
    Shutdown();
}

bool X11InputEngine::Initialize() {
#if defined(HAVE_XLIB)
    if (m_initialized) return true;

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        return false;
    }

    m_display = static_cast<void*>(dpy);
    m_initialized = true;

#if defined(HAVE_XTEST)
    int eventBase, errorBase, majorVersion, minorVersion;
    if (XTestQueryExtension(dpy, &eventBase, &errorBase, &majorVersion, &minorVersion)) {
        m_hasXTest = true;
    }
#endif

    return true;
#else
    return false;
#endif
}

void X11InputEngine::Shutdown() {
#if defined(HAVE_XLIB)
    if (m_display) {
        XCloseDisplay(static_cast<Display*>(m_display));
        m_display = nullptr;
    }
#endif
    m_initialized = false;
    m_hasXTest = false;
}

bool X11InputEngine::IsAvailable() const {
    return m_initialized;
}

void X11InputEngine::FakeKeyEvent(unsigned int keycode, bool isDown) {
#if defined(HAVE_XLIB) && defined(HAVE_XTEST)
    if (!m_display || !m_hasXTest) return;
    Display* dpy = static_cast<Display*>(m_display);
    XTestFakeKeyEvent(dpy, keycode, isDown ? True : False, CurrentTime);
    XFlush(dpy);
#else
    (void)keycode; (void)isDown;
#endif
}

void X11InputEngine::EmitBackspacesAndChars(size_t backspaces, const std::wstring& toSend) {
#if defined(HAVE_XLIB)
    if (!m_display) {
        if (!Initialize()) return;
    }

    Display* dpy = static_cast<Display*>(m_display);

#if defined(HAVE_XTEST)
    if (m_hasXTest) {
        KeyCode backspaceCode = XKeysymToKeycode(dpy, XK_BackSpace);
        for (size_t i = 0; i < backspaces; ++i) {
            XTestFakeKeyEvent(dpy, backspaceCode, True, CurrentTime);
            XTestFakeKeyEvent(dpy, backspaceCode, False, CurrentTime);
        }
        XFlush(dpy);
    }
#endif

    // For Unicode text, UTF-8 conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Text = converter.to_bytes(toSend);

    // Emits keystrokes or dispatches to client
    XFlush(dpy);
#else
    (void)backspaces; (void)toSend;
#endif
}

bijoy::core::NativeWindowHandle X11InputEngine::GetActiveWindow() {
#if defined(HAVE_XLIB)
    if (!m_display) {
        if (!Initialize()) return 0;
    }

    Display* dpy = static_cast<Display*>(m_display);
    Atom netActiveWindow = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", True);
    if (netActiveWindow == None) {
        return 0;
    }

    Atom actualType;
    int actualFormat;
    unsigned long nItems, bytesAfter;
    unsigned char* prop = nullptr;

    Window root = DefaultRootWindow(dpy);
    if (XGetWindowProperty(dpy, root, netActiveWindow, 0, 1, False,
                           XA_WINDOW, &actualType, &actualFormat,
                           &nItems, &bytesAfter, &prop) == Success) {
        if (prop && nItems > 0) {
            Window active = *reinterpret_cast<Window*>(prop);
            XFree(prop);
            return static_cast<bijoy::core::NativeWindowHandle>(active);
        }
    }
#endif
    return 0;
}

bool X11InputEngine::GrabGlobalHotkey(int keycode, unsigned int modifiers) {
#if defined(HAVE_XLIB)
    if (!m_display) {
        if (!Initialize()) return false;
    }
    Display* dpy = static_cast<Display*>(m_display);
    Window root = DefaultRootWindow(dpy);
    XGrabKey(dpy, keycode, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    XFlush(dpy);
    return true;
#else
    (void)keycode; (void)modifiers;
    return false;
#endif
}

void X11InputEngine::UngrabGlobalHotkeys() {
#if defined(HAVE_XLIB)
    if (m_display) {
        Display* dpy = static_cast<Display*>(m_display);
        Window root = DefaultRootWindow(dpy);
        XUngrabKey(dpy, AnyKey, AnyModifier, root);
        XFlush(dpy);
    }
#endif
}

} // namespace bijoy::platform::linux
#endif

