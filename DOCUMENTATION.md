# Omor Ekushe (অমর একুশে)

**Omor Ekushe** is a lightweight, high-performance Bengali keyboard layout manager designed for Windows. It provides a seamless typing experience by allowing users to switch between different Bengali keyboard layouts (like Bijoy) and English effortlessly.

---

## Features

- **Multi-Layout Support**: Supports multiple Bengali keyboard layouts. Layouts are loaded dynamically from XML-based format files.
- **Window-Specific Binding**: Automatically remembers and applies different layouts to different active windows.
- **System Tray Integration**: Minimizes to the system tray to stay out of the way while remaining accessible.
- **Global Keyboard Hooking**: Intercepts keystrokes at a low level to provide consistent mapping across all applications.
- **Keyboard Shortcuts**: Switch between layouts quickly using customizable global shortcuts (e.g., Ctrl+Alt+B).
- **Layout Discovery**: Automatically searches for and loads available layout files in the application directory.
- **Customizable Aesthetics**: Support for semi-transparent background images (glassmorphism-lite) for the main control window.
- **Layout Editor Support**: Integrated shortcut to open a dedicated Layout Editor for creating/modifying keyboard mappings.

---

## Disadvantages / Limitations

- **Platform Dependent**: Currently only supports Windows (Win32 API). Not compatible with macOS or Linux.
- **Work in Progress**: The "Options" dialog is currently a placeholder and not yet implemented.
- **Resource Dependency**: Requires specific data files (icons, backgrounds, and layout XMLs) to be present in the `data` directory.
- **Phonetic Typing**: Currently focused on fixed mapping layouts; advanced phonetic engine (like Avro) is not yet fully implemented.

---

## Upcoming Features (Roadmap)

- **[ ] Options Dialog**: A full-featured settings menu to customize shortcuts, transparency, and startup behavior.
- **[ ] Phonetic Engine**: Support for phonetic Bengali typing (write Bengali using English alphabet).
- **[ ] Layout Sharing**: Export and import layouts easily through the UI.
- **[ ] Cross-Platform Support**: Plans to port the core engine to Linux and macOS using a cross-platform GUI framework.
- **[ ] Auto-Update**: Built-in mechanism to check for and install new layouts or software updates.

---

## Technical Details

- **Language**: C++17
- **Build System**: CMake 3.14+
- **Platform**: Windows (MinGW/MSVC)
- **Dependencies**: 
  - `stb_image` (embedded for image loading)
  - `Win32 API` (Native UI and Keyboard Hooks)

---

## How to Use

1. **Launch**: Run `OmorEkushe.exe`. The small control window will appear at the top of your screen.
2. **Select Layout**: Use the dropdown menu to choose your desired Bengali layout or "English".
3. **Switch Hotkey**: Press the configured hotkey (default is usually Ctrl+Alt+B) to toggle layouts without touching the mouse.
4. **Minimize**: Click the `_` button to hide the window to the system tray. Double-click the tray icon to bring it back.
