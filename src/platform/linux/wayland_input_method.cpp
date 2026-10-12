#if !defined(_WIN32) && !defined(__APPLE__)
#include "platform/linux/wayland_input_method.h"
#include "core/phonetic_engine.h"
#include <iostream>

namespace {

inline std::string WStringToUtf8(const std::wstring& wstr) {
    std::string out;
    for (wchar_t wc : wstr) {
        uint32_t cp = static_cast<uint32_t>(wc);
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

} // namespace

namespace bijoy::platform::linux {

WaylandInputMethodEngine& WaylandInputMethodEngine::Instance() {
    static WaylandInputMethodEngine instance;
    return instance;
}

WaylandInputMethodEngine::WaylandInputMethodEngine() = default;
WaylandInputMethodEngine::~WaylandInputMethodEngine() {
    Shutdown();
}

bool WaylandInputMethodEngine::Initialize() {
    m_initialized = true;
    return true;
}

void WaylandInputMethodEngine::Shutdown() {
    m_initialized = false;
    m_currentPreedit.clear();
}

bool WaylandInputMethodEngine::IsAvailable() const {
    return m_initialized;
}

void WaylandInputMethodEngine::CommitText(const std::wstring& utf16Text) {
    if (!m_initialized) return;
    std::string utf8 = WStringToUtf8(utf16Text);
    // Wayland text-input-v3: zwp_text_input_v3_commit_string
    // IBus: ibus_engine_commit_text
    m_currentPreedit.clear();
}

void WaylandInputMethodEngine::DeleteSurroundingText(int offset, unsigned int length) {
    if (!m_initialized) return;
    // Wayland text-input-v3: zwp_text_input_v3_delete_surrounding_text
    // IBus: ibus_engine_delete_surrounding_text
    (void)offset;
    (void)length;
}

void WaylandInputMethodEngine::SetPreeditText(const std::wstring& preedit, int cursorPosition) {
    m_currentPreedit = preedit;
    (void)cursorPosition;
    // Wayland text-input-v3: zwp_text_input_v3_set_preedit_string
    // IBus: ibus_engine_update_preedit_text
}

bool WaylandInputMethodEngine::ProcessKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state) {
    if (!m_initialized) return false;

    // Check for F12 toggle or layout switch
    constexpr unsigned int KEY_F12 = 0xffc9;
    if (keyval == KEY_F12) {
        // Toggle Phonetic
        return true;
    }

    // Pass ASCII keys through the Phonetic Engine
    if (keyval >= 0x20 && keyval <= 0x7E) {
        char ch = static_cast<char>(keyval);
        // Dispatch to phonetic engine
        return bijoy::core::g_phoneticEngine.ProcessCharacter(ch, 0);
    }

    if (keyval == 0xff08) { // BackSpace
        return bijoy::core::g_phoneticEngine.ProcessBackspace(0);
    }

    (void)keycode;
    (void)state;
    return false;
}

} // namespace bijoy::platform::linux
#endif

