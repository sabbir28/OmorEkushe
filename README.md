# Omor Ekushe – C/C++ Port

Native Win32 C/C++ port of the Omor Ekushe keyboard layout application (originally C#/.NET). It provides multi-layout input with shortcut switching and per-window layout memory.

---

## Table of Contents

1. [Overview](#overview)
2. [Requirements](#requirements)
3. [Building](#building)
4. [Project Structure](#project-structure)
5. [Usage](#usage)
6. [Configuration & Data](#configuration--data)
7. [API Overview](#api-overview)
8. [Replatforming Strategy](#replatforming-strategy)
9. [Missing Functionality vs C#](#missing-functionality-vs-c)

---

## Overview

- **Purpose:** Run in the system tray, load keyboard layouts from XML files in a `Layouts` folder, and inject converted key output into the active window using a low-level keyboard hook.
- **Entry flow:** Registration dialog (activation code) → main window (optional) → tray icon. Layouts are loaded from `Layouts\` (and subfolders) next to the executable.
- **Tech:** C++17, Win32 API, no .NET. Build with CMake (Visual Studio or MinGW).

---

## Requirements

- **OS:** Windows 10/11 (or compatible)
- **Build:**
  - **Option A:** Visual Studio 2019/2022 with “Desktop development with C++” and **Win32** (x86) support
  - **Option B:** MinGW-w64 with `g++` and `windres` (or CMake’s default generator)
- **CMake:** 3.14 or newer

---

## Building

### From repository root

You can use the root CMake build system to build only the C++ part:

```batch
cmake --preset windows-default
cmake --build --preset build-cpp
```

### Inside `OmorEkusheCpp/`

### One-command build script (Windows)

```batch
cd OmorEkusheCpp
build.bat
```

Optional arguments:

```batch
build.bat [Configuration] [Platform] [Generator]
build.bat Debug Win32 "Visual Studio 17 2022"
```

### Visual Studio (Win32)

```batch
cd OmorEkusheCpp
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A Win32
cmake --build . --config Release
```

Executable: `build\Release\OmorEkushe.exe` (or `Debug\OmorEkushe.exe` for Debug).

### Command line (MSVC)

```batch
cd OmorEkusheCpp\build
cmake .. -G "Visual Studio 17 2022" -A Win32
msbuild OmorEkushe.sln /p:Configuration=Release /p:Platform=Win32
```

### MinGW

If your environment does not provide a resource compiler (`rc.exe`/`windres`), configure CMake with `-DBIJOY_INCLUDE_RESOURCES=OFF`.


```batch
cd OmorEkusheCpp
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build .
```

---

## Project Structure

```
OmorEkusheCpp/
├── CMakeLists.txt          # CMake project and executable
├── README.md                # This file
├── DOCUMENTATION.md         # Detailed module and design notes
├── REPLATFORMING_STRATEGY.md# Migration rationale and architecture direction
├── MISSING_FUNCTIONALITY.md # Gap list vs original C# app
└── src/
    ├── resource.h           # Resource IDs (dialogs, controls, menu)
    ├── app.rc               # Dialog and (optional) icon resources
    ├── main.cpp             # WinMain, init, message loop
    ├── native_api.h/cpp      # SendInput, INPUT, KEYBDINPUT, DoKeyBoard
    ├── keyboard_hook.h/cpp  # Low-level keyboard hook, layout switch & key conversion
    ├── layout.h/cpp         # Layout/LayoutShortcut/KeyMapping, XML load
    ├── handle_info.h/cpp    # Per-window layout (HWND → Layout*)
    ├── app_state.h/cpp      # Global layout list, current layout index
    ├── my_functions.h/cpp   # Registry, path, machine ID, encrypt
    ├── find_layouts.h/cpp   # Scan Layouts folder, load XML layouts
    ├── dlg_registration.h/cpp # Registration dialog (activation code)
    └── dlg_main.h/cpp       # Tray icon, popup menu (layouts, Show, Exit)
```

---

## Usage

1. **First run:** Place layout XML files in a `Layouts` folder next to the executable (same structure as the C# app). The app will show the registration dialog if the saved password is not present.
2. **Activation:** Enter the activation code in the four boxes (same logic as C#). On success, the dialog closes and the app continues.
3. **Tray:** The app minimizes to the system tray. Right-click the tray icon for:
   - **Show** – show the main window (if implemented)
   - **Layout names** – switch active layout
   - **Exit** – quit the app
4. **Shortcuts:** Use the shortcut defined in each layout’s XML (e.g. Ctrl+Alt+Key) to switch that layout on/off for the foreground window.
5. **Key conversion:** When a layout is active, keys are translated according to the layout’s Normal/Shift mappings and injected via `SendInput`.

---

## Configuration & Data

- **Registry:**
  - Password (activation): `HKEY_CURRENT_USER\SOFTWARE\OmorEkushe\Options` → `Password`
  - The C# app also used `SOFTWARE\BijoyEkushe\Options` for DefaultLayout, TrayMode, Position, LAM, ApplicationMode; the C++ port does not yet persist all of these (see [MISSING_FUNCTIONALITY.md](MISSING_FUNCTIONALITY.md)).
- **Paths:**
  - Layouts: `%AppDir%\Layouts\` (and subfolders), `*.xml`
  - Icons (C#): `%AppDir%\Icons\`; C++ tray uses a default icon unless you add an `.ico` and reference it in `app.rc`.

---

## API Overview

| Module           | Role |
|------------------|------|
| **main.cpp**     | `wWinMain`: init common controls, load layouts, install hook, show registration then main/tray, message loop, uninstall hook on exit. |
| **native_api**   | `SendInput` wrapper; `DoKeyBoard(flags, scanCode)` and `DoKeyBoardVk(flags, vk)` for key injection (including Unicode). |
| **keyboard_hook**| `KeyboardHook_Install` / `Uninstall`; `KeyboardHook_SetLayoutsReady`. Hook handles shortcut detection and layout switching, and key translation using current layout’s `key` map. |
| **layout**       | `Layout` (name, shortcut, key map, juk map); `loadFromFile(path)` / `updateFromFile()`; simple XML parsing for KeyLayout format. |
| **handle_info**  | `AddInfo(hwnd, layout)`, `FindInfo(hwnd)`, `RemoveInfo(hwnd)` to store per-window layout. |
| **app_state**    | `g_layouts`, `g_currentLayoutIndex`, `g_comLayoutSelectedIndex`; get/set current layout and resolve layout by index. |
| **my_functions** | Registry read/write for password; `GetMachineId`, `VerifyPath`, `Encrypt` (for activation). |
| **find_layouts** | `GetAppDirectory()`, `FindLayouts(layouts, appDir)` to discover and load all layout XMLs. |
| **dlg_registration** | Modal registration dialog: machine ID display, 4 code fields, OK/Cancel; validates against stored password and registry. |
| **dlg_main**     | Creates the main window (hidden by default), tray icon, and popup menu with layout items and Show/Exit. |

---

## Replatforming Strategy

This repository is part of a deliberate migration from an abandoned C# desktop stack to a maintainable native engine:

- Recover legacy behavior through decompilation and analysis of the original binaries.
- Extract core typing algorithms (key mapping, encoding, shortcut/IME behavior).
- Re-implement those rules in deterministic C/C++ modules decoupled from UI concerns.
- Keep Win32-specific integration at the platform boundary so future UIs/bindings can reuse the same engine.

For full migration rationale and phased architecture, see **[REPLATFORMING_STRATEGY.md](REPLATFORMING_STRATEGY.md)**.

---

## Missing Functionality vs C#

See **[MISSING_FUNCTIONALITY.md](MISSING_FUNCTIONALITY.md)** for a detailed list of features present in the original C# app but not yet implemented (or simplified) in the C++ port, and **[DOCUMENTATION.md](DOCUMENTATION.md)** for more technical and design notes.

---

## License

This project is released under the **Sabbir Legacy License (MIT)**. See **[LICENSE](LICENSE)**.
