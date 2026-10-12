# Omor Ekushe (অমর একুশে) — Technical Documentation

**Omor Ekushe** is a lightweight, high-performance Bengali keyboard engine designed for Windows. It provides an ultra-responsive native typing experience combining dynamic XML layout mapping (Bijoy, Unijoy, National) with an Avro-compatible phonetic transliteration engine.

---

## Architecture Overview

Omor Ekushe is built on three core pillars:

1. **Kernel-Level Input Interception (`WH_KEYBOARD_LL`)**:
   Keystrokes are captured via a Windows low-level keyboard hook before applications process them. Depending on the active mode, characters are translated and emitted using `SendInput` with Unicode glyphs.

2. **Dual Typing Engine**:
   - **Fixed Layout Engine (`layout.cpp`)**: Parses XML layout files on the fly and matches virtual key codes and modifiers to UTF-16 Bengali characters.
   - **Phonetic Engine (`phonetic_engine.cpp` & `phonetic_parser.cpp`)**: Implements an Avro-standard phonetic Trie parser with vowel-to-Kar conversions, consonant clusters, conjunct resolution, and **smart dynamic backspacing**.

3. **Per-Window Layout Context (`window_binding.cpp`)**:
   Automatically tracks window focus transitions and maintains the user's preferred layout (Bangla vs. English) per application window.

4. **Options & Configuration UI (`options_dialog.cpp`)**:
   Native Win32 settings modal allowing users to configure overlay opacity (20%-100%), auto-start with Windows, and global hotkeys.

---

## Global Shortcuts & Hotkeys

| Action | Default Hotkey | Configurable | Behavior |
| :--- | :--- | :--- | :--- |
| **Cycle Layout** | <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>B</kbd> | Yes (Options Dialog) | Switches to the next available XML layout or English |
| **Toggle Phonetic** | <kbd>F12</kbd> (or <kbd>Ctrl+Alt+P</kbd>) | Yes (Options Dialog) | Instant toggle between Phonetic typing and English |
| **Minimize to Tray** | <kbd>Esc</kbd> or `_` button | N/A | Hides top control bar to system notification area |
| **Restore Bar** | Double-Click Tray Icon | N/A | Restores the floating control bar |
| **Open Options** | Right-Click Tray &rarr; Options | N/A | Opens native settings dialog |
| **Exit** | Right-Click Tray &rarr; Exit | N/A | Detaches hook service and terminates process cleanly |

---

## Inno Setup & Silent Upgrades

The application installer is configured in `setup/OmorEkushe.iss` and generated automatically via GitHub Actions CI:
- **Clean In-Place Upgrades**: Uses `CloseApplications=yes` and `RestartApplications=yes` to detect and safely close running instances during updates.
- **Silent Installation Flag**: Supports unattended deployments:
  ```powershell
  OmorEkushe-Setup.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
  ```

---

## Registry Keys

Settings are stored under `HKEY_CURRENT_USER\Software\OmorEkushe`:
- `Opacity`: Integer DWORD representing overlay alpha (50-255).
- `AutoStart`: DWORD (0 or 1). Also mirrored to `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\OmorEkushe`.
- `ShortcutModifiers`: Bitmask for layout toggle modifier keys.
- `ShortcutKey`: Virtual key code for layout toggle.
- `PhoneticModifiers`: Bitmask for phonetic modifier keys.
- `PhoneticKey`: Virtual key code for phonetic toggle (default: `VK_F12`).

To reset settings:
```bat
reg delete "HKCU\Software\OmorEkushe" /f
```

---

## Visionary Future Roadmap

- [ ] **AI-Powered Predictive Word Suggestions**: Non-intrusive floating candidate bar powered by n-gram frequency dictionaries.
- [ ] **Cloud Layout Marketplace**: In-app browser to discover and download user-contributed XML layout maps.
- [ ] **Background Auto-Updater Daemon**: Seamless background downloader utilizing silent Inno Setup updates.
- [ ] **Cross-Platform Linux & macOS Ports**: Native Wayland/X11 and macOS input method integration.
- [ ] **Phonetic Spellchecker**: Real-time spell correction suggestions for Bengali text fields.
- [ ] **Voice-to-Bangla Speech Typing**: Direct voice recognition input.
