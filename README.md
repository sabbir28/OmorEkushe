# Omor Ekushe (অমর একুশে)

> **A lightweight, high-performance, 100% native Win32 Bengali (Bangla) Keyboard Layout Manager & Engine for Windows.**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/sabbir28/OmorEkushe)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Win32-blue.svg)](https://github.com/sabbir28/OmorEkushe)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)](https://github.com/sabbir28/OmorEkushe)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![CMake](https://img.shields.io/badge/CMake-3.14%2B-informational.svg)](CMakeLists.txt)

---

## Overview

**Omor Ekushe (অমর একুশে)** is a modern Windows application designed to bring effortless, high-speed Bengali (Bangla) typing to native desktop applications. Named in honor of the Bengali Language Movement, Omor Ekushe provides a seamless layout switching engine powered by low-level Windows keyboard hooks, dynamic XML-driven layout configuration, and per-window layout persistence.

Engineered using **pure native Win32 C++17**, Omor Ekushe operates with virtually zero runtime overhead and an ultra-minimal memory footprint (~5MB RAM), delivering instant responsiveness across all Windows versions.

---

## Key Features

- **100% Native Win32 Core**: Built without heavy UI frameworks (Electron/Qt), guaranteeing maximum speed, instant startup, and tiny resource usage.
- **Dynamic Multi-Layout Engine**: Load any Bengali keyboard layout dynamically from standard XML files (e.g., Bijoy, Unijoy, National, Custom).
- **Per-Window Layout Memory**: Automatically remembers and applies your active typing layout (Bangla/English) on a per-window basis as you switch focus.
- **Low-Level Keystroke Interception**: Utilizes Windows low-level keyboard hooks (`WH_KEYBOARD_LL`) for high-precision, lag-free character translation across all applications.
- **Acrylic Glassmorphism Bar**: Modern control overlay with custom background image support and semi-transparency.
- **System Tray Integration**: Minimizes silently to the notification area with full context menu controls and hotkey access.
- **Integrated Spelling Checker**: Built-in sub-module (`SpellingChecker`) for real-time Bengali spellchecking and dictionary lookup.
- **Cloud NetClient Sub-Module**: Network client (`NetClient`) for layout synchronization, online updates, and layout sharing.
- **Optional Layout Editor**: Interactive GUI editor (`LayoutEditor`) built with GLFW, OpenGL, and Dear ImGui for designing and testing custom XML keyboard maps.
- **Embedded Installer Bundler**: Includes an internal resource bundler (`bundler`) and standalone installer executable builder.

---

## Project Architecture

```
OmorEkushe/
├── src/                          # Main Application Source Code
│   ├── app/                      # Application Entry Point & Core Lifecycle (main.cpp, application.cpp)
│   ├── core/                     # App State, Hooking Service, Layout Engine, Window Bindings
│   ├── error/                    # Crash Diagnostics & Exception Handler
│   ├── platform/                 # Platform-specific native UI implementations
│   │   └── windows/              # Win32 Main Window, Options Overlay, Tray, Splash Screen, Resources
│   └── utils/                    # System Utilities & Helper Functions
├── include/                      # Core C++ Header Files
├── LayoutEditor/                 # Optional ImGui/OpenGL Graphical Layout Editor
├── NetClient/                    # Network Synchronization & Cloud Layout Client
├── SpellingChecker/              # Bengali Spell Checking Engine & Dictionary Core
├── Installer/                    # Bundler Utility & Self-contained Win32 Installer Setup
├── CMakeLists.txt                # Unified Root CMake Build Script
├── DOCUMENTATION.md              # Technical Architecture & Usage Notes
├── LICENSE                       # MIT License
└── remove_registry.bat           # Utility script to clean registry settings
```

---

## Build & Installation Guide

### Prerequisites

To compile Omor Ekushe from source, ensure you have the following tools installed:

- **Operating System**: Windows 10 / 11 (or Windows 7+)
- **Compiler**: 
  - **MSVC**: Visual Studio 2019 / 2022 (with C++ Desktop Workload) **OR**
  - **MinGW-w64**: GCC 9.0+ (`g++`) with POSIX threads
- **Build System**: [CMake 3.14+](https://cmake.org/download/)

---

### Building with CMake (MSVC / Visual Studio)

```powershell
# 1. Clone the repository
git clone https://github.com/sabbir28/OmorEkushe.git
cd OmorEkushe

# 2. Generate build files for Visual Studio (x64)
cmake -B build -A x64

# 3. Compile Release executable
cmake --build build --config Release
```

The compiled binaries and data assets will be generated in `build/bin/`.

---

### Building with MinGW-w64

```bash
# 1. Generate MinGW Makefiles
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 2. Build executable
cmake --build build
```

---

### Building Optional Layout Editor

The interactive layout editor uses OpenGL and GLFW (automatically fetched via CMake FetchContent):

```powershell
cmake -B build -DBIJOY_BUILD_LAYOUT_EDITOR=ON
cmake --build build --config Release
```

---

### Generating Installer Setup Executable

To build the self-contained single-file setup installer:

```powershell
# Build all targets including Installer
cmake --build build --target Installer --config Release
```

The output installer executable will be placed in `build/bin/Installer.exe`.

---

## Usage & Default Hotkeys

1. **Launch**: Double-click `OmorEkushe.exe`. The compact control window appears at the top of your primary display.
2. **Toggle Layout**: Press <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>B</kbd> (or your configured shortcut) to instantly switch between **Bangla** and **English** modes.
3. **Select Layout**: Select your desired layout from the dropdown menu (e.g., Bijoy, Unijoy, National).
4. **Minimize to Tray**: Click the minimize button (`_`) or press <kbd>Esc</kbd>. The app will continue running silently in the system tray. Double-click the tray icon to restore.
5. **Exit**: Right-click the system tray icon and select **Exit**, or close the main bar.

---

## Custom Layout XML Format

Omor Ekushe dynamically parses XML layout files stored in the `data/` directory. You can create custom layouts by defining key map files following this format:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Layout name="CustomBijoy" author="Sabbir" version="1.0">
  <KeyMap vk="0x41"> <!-- Virtual Key Code for 'A' -->
    <Normal>া</Normal>
    <Shift>অ</Shift>
    <AltGr>্</AltGr>
  </KeyMap>
  <KeyMap vk="0x42"> <!-- Virtual Key Code for 'B' -->
    <Normal>ন</Normal>
    <Shift>ণ</Shift>
  </KeyMap>
</Layout>
```

---

## Roadmap & Future Enhancements

- [x] Pure Win32 Native Application Core
- [x] Dynamic XML Layout Discovery Engine
- [x] Per-Window Active Layout Memory
- [x] System Tray Integration & Glassmorphism Overlay
- [x] Integrated Spelling Checker Core
- [x] Dedicated Setup Installer Generator
- [ ] **Full Options & Configuration UI**: Settings menu for custom hotkeys, overlay opacity, and auto-start.
- [ ] **Advanced Phonetic Engine**: Write Bengali phonetically using English characters (Avro-style layout parsing).
- [ ] **Cloud Layout Marketplace**: Download and share layout XML files online via `NetClient`.
- [ ] **Cross-Platform Linux Core**: Linux port using X11 / Wayland native hooks.

---

## Contributing

Contributions are welcome! Whether you are reporting a bug, proposing a feature, or submitting a new Bengali layout XML file, please read our [CONTRIBUTING.md](CONTRIBUTING.md) guide and follow our [Code of Conduct](CODE_OF_CONDUCT.md).

1. Fork the Repository
2. Create a Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for more information.

---

## Acknowledgments & Credits

- Created in memory of the Language Movement Heroes of February 21, 1952 (**অমর একুশে**).
- Developed by **Sabbir Legacy Contributors**.
- Uses [stb_image](https://github.com/nothings/stb) for light Win32 image decoding.
- Uses [Dear ImGui](https://github.com/ocornut/imgui) & [GLFW](https://www.glfw.org/) for the optional Layout Editor.
