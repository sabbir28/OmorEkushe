# Omor Ekushe (অমর একুশে)

> **A modern, lightweight, 100% native Win32 Bengali (Bangla) Keyboard Engine for Windows.**  
> Featuring an **Avro-compatible Phonetic Transliteration Engine**, **Dynamic XML Layout Maps**, a **Full Configuration & Options UI**, and automated **Inno Setup** distribution.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/sabbir28/OmorEkushe)
[![GitHub Release](https://img.shields.io/github/v/release/sabbir28/OmorEkushe?color=emerald&label=Release)](https://github.com/sabbir28/OmorEkushe/releases/latest)
[![Platform](https://img.shields.io/badge/platform-Windows%2010%20%2F%2011%20(x64)-blue.svg)](https://github.com/sabbir28/OmorEkushe)
[![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)](https://github.com/sabbir28/OmorEkushe)
[![Installer](https://img.shields.io/badge/installer-Inno%20Setup%20CI-teal.svg)](packaging/windows/OmorEkushe.iss)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![RAM Usage](https://img.shields.io/badge/RAM-~5%20MB-red.svg)](#performance)

---

## What's New

- **Rust & Hardware SIMD/Assembly Acceleration Core**:
  - Critical phonetic transliteration hot paths and dynamic diff backspacing rewritten with **Rust and x86_64 SIMD vectorization (SSE2/AVX)** and **inline assembly bit-scans (`bsf`)**.
  - Replaced heap-allocating string searches with single-cycle $O(1)$ 256-byte static classification lookup tables.
  - Multi-threaded or single-keystroke dynamic diffing compares 8 to 16 UTF-16 code units per vector instruction cycle, achieving zero perceptible latency.
- **Avro-Compatible Phonetic Transliteration Engine**:
  - Type Bengali phonetically using standard English keyboard inputs (e.g., `ami` &rarr; `আমি`, `bangla` &rarr; `বাংলা`, `shadhinota` &rarr; `স্বাধীনতা`).
  - Powered by a native C++ Trie parser, complex conjunct resolution, vowel-to-Kar contextual transformation, and **smart dynamic backspacing** that edits already-committed glyphs seamlessly.
  - Dedicated **<kbd>F12</kbd>** toggle hotkey for instantaneous switching between English and Phonetic typing.
- **Smart Text Expander & Date/Currency Macro Engine**:
  - Automatically expands trigger prefixes into formatted Bengali text:
    - Shorthands: `//om` &rarr; `অমর একুশে`, `//bd` &rarr; `বাংলাদেশ`, `//dh` &rarr; `ঢাকা`, `//slm` &rarr; `আসসালামু আলাইকুম`.
    - Localized Date & Time: `//date` &rarr; `১১ নভেম্বর ২০২৬`, `//time` &rarr; `১২:০০ অপরাহ্ন`.
    - Numeric Currency to Bengali Words: amounts followed by `/=` (e.g. `1500/=`) expand into words: `এক হাজার পাঁচ শত টাকা মাত্র`.
- **Predictive Autocomplete Suggestion Engine**:
  - Real-time candidate completions for active Bengali prefixes.
  - Press <kbd>Tab</kbd> during phonetic typing to immediately auto-complete to the top candidate suggestion.
- **Full Native Configuration & Options Dialog**:
  - Access via system tray right-click or top bar menu &rarr; **Options...**
  - **Live Glassmorphism Opacity Slider**: Adjust overlay bar transparency from 20% to 100% with real-time visual feedback.
  - **Auto-Start with Windows**: Toggle seamless startup via `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`.
  - **Customizable Global Hotkeys**: Configure distinct keyboard shortcuts for Layout Cycle (<kbd>Ctrl+Alt+B</kbd>), Phonetic Toggle (<kbd>F12</kbd> or <kbd>Ctrl+Alt+P</kbd>).
- **Automated Inno Setup Architecture (Zero Local Overhead)**:
  - Replaced internal C++ binary bundler with an industry-standard **Inno Setup** packaging pipeline (`packaging/windows/OmorEkushe.iss`).
  - Developers compile pure C++ application binaries locally; GitHub Actions CI automatically builds and signs the official **`OmorEkushe-Setup.exe`** and **`OmorEkushe-Portable.zip`**.
- **Smart Seamless Updates**:
  - Upgraded installer detects running instances of `OmorEkushe.exe`, gracefully closes them, applies the update, and relaunches the application.
  - Supports silent automation switches: `OmorEkushe-Setup.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART`.

---

## Table of Contents

- [Overview](#overview)
- [Key Features](#key-features)
- [Phonetic Typing Cheat Sheet](#phonetic-typing-cheat-sheet)
- [System Architecture](#system-architecture)
- [Building from Source](#building-from-source)
- [Installation & Distribution](#installation--distribution)
- [Configuration & Settings](#configuration--settings)
- [Visionary Future Roadmap](#visionary-future-roadmap)
- [Contributing](#contributing)
- [License & Acknowledgments](#license--acknowledgments)

---

## Overview

**Omor Ekushe (অমর একুশে)** is a high-speed, native Windows desktop application engineered to deliver effortless Bengali (Bangla) typing across all modern applications. Named in eternal honor of the Bengali Language Movement of February 21, 1952, Omor Ekushe combines the best of both typing worlds:

1. **Fixed Layouts**: Classic and Unicode layouts (Bijoy, Unijoy, National, Custom) dynamically loaded from user-editable XML files.
2. **Phonetic Transliteration**: Type phonetic English sounds that naturally convert to beautiful Bengali script with complete Avro grammar compatibility.

Built using **100% native Win32 API and C++17**, Omor Ekushe operates with zero Electron/web runtime bloat, consuming **~5 MB of RAM** and providing sub-millisecond keystroke translation.

---

## Key Features

- **Dual-Mode Typing Core**: Seamlessly switch between fixed XML layouts (Bijoy/Unijoy) and the phonetic Avro transliteration engine.
- **Low-Level Windows Hooking**: Uses `WH_KEYBOARD_LL` and Unicode `SendInput` for universal compatibility across browsers, Office suites, IDEs, terminals, and games.
- **Dynamic Diff Backspacing**: Phonetic engine dynamically computes Unicode character differences and emits minimal backspaces to transform characters on the fly as you type syllables.
- **Per-Window Layout Memory**: Remembers whether you were typing in Bangla or English on each application window independently.
- **Glassmorphism Acrylic Overlay**: Elegant, draggable floating control bar with adjustable real-time transparency (20% &ndash; 100%).
- **System Tray Notification Area**: Silently minimizes to tray with full context menu controls, options dialog, update checker, and layout quick-selection.
- **Integrated Spelling Checker**: Built-in `SpellingChecker` sub-module for Bengali word verification and candidate suggestions.
- **Optional Dear ImGui Layout Editor**: Interactive OpenGL/ImGui graphical tool (`LayoutEditor`) for designing custom XML keymaps visually.

---

## Phonetic Typing Cheat Sheet

With **Phonetic Mode** active (<kbd>F12</kbd>), type English phonetically according to the Avro standard:

| English Input | Bengali Output | Example Words |
| :--- | :--- | :--- |
| `a`, `A` / `aa` | অ / আ | `ami` &rarr; আমি, `amra` &rarr; আমরা |
| `i`, `I` / `ee` | ই / ঈ | `ilish` &rarr; ইলিশ, `eid` &rarr; ঈদ |
| `u`, `U` / `oo` | উ / ঊ | `utshob` &rarr; উৎসব, `ushar` &rarr; ঊষার |
| `e`, `E` / `ai` | এ / ঐ | `ek` &rarr; এক, `oikya` &rarr; ঐক্য |
| `o`, `O` / `ou` | ও / ঔ | `olonkar` &rarr; অলংকার, `oushodh` &rarr; ঔষধ |
| `k`, `kh`, `g`, `gh`, `Ng` | ক, খ, গ, ঘ, ঙ | `kotha` &rarr; কথা, `gaan` &rarr; গান, `rong` &rarr; রং |
| `c`, `ch`, `j`, `jh`, `NG` | চ, ছ, জ, ঝ, ঞ | `chad` &rarr; চাঁদ, `jol` &rarr; জল, `jhor` &rarr; ঝড় |
| `T`, `Th`, `D`, `Dh`, `N` | ট, ঠ, ড, ঢ, ণ | `Taka` &rarr; টাকা, `Dhab` &rarr; ডাব, `ghoNTa` &rarr; ঘণ্টা |
| `t`, `th`, `d`, `dh`, `n` | ত, থ, দ, ধ, ন | `tumi` &rarr; তুমি, `desh` &rarr; দেশ, `nodi` &rarr; নদী |
| `p`, `ph`/`f`, `b`, `bh`/`v`, `m` | প, ফ, ব, ভ, ম | `pani` &rarr; পানি, `ful` &rarr; ফুল, `bhalobasha` &rarr; ভালোবাসা |
| `r`, `l`, `sh`, `Sh`, `s`, `h` | র, ল, শ, ষ, স, হ | `shadhinota` &rarr; স্বাধীনতা, `shanti` &rarr; শান্তি, `shurjo` &rarr; সূর্য |
| `R`, `Rh`, `y`, `Y` | ড়, ঢ়, য়, য় | `baRi` &rarr; বাড়ি, `aShaRh` &rarr; আষাঢ়, `doya` &rarr; দয়া |
| `.` (Period) | । (দাঁড়ি) | `ami banglay gan gai.` &rarr; আমি বাংলায় গান গাই। |
| `kkh`, `jng`, `nd`, `nt`, `mps` | ক্ষ, জ্ঞ, ন্দ, ন্ত, ম্প | `shikkha` &rarr; শিক্ষা, `gyan` &rarr; জ্ঞান, `anondo` &rarr; আনন্দ |

---

## System Architecture

```
OmorEkushe/
├── .github/                      # Automated CI/CD Automation Workflows
│   ├── workflows/                # Multi-compiler CI & Inno Setup automated release
│   └── ISSUE_TEMPLATE/           # Structured bug reports & feature requests
├── cmake/                        # Modular CMake helper scripts & warning profiles
│   └── CompilerWarnings.cmake    # Standardized compiler warnings (MSVC, GCC, Clang)
├── data/                         # Shipped Canonical Assets (never inside build/!)
│   ├── Icons/                    # Application and layout-specific .ico icons
│   └── Layouts/                  # Shipped XML Keymaps (Classic, Phonetic, Unicode)
├── docs/                         # Modern Project Documentation & GitHub Pages Site
├── include/                      # Public C++ Header Files
│   ├── app/                      # Application lifecycle headers
│   ├── core/                     # Transliteration, hooking, state, layout headers
│   ├── error/                    # Crash handling and RAII utilities
│   ├── lib/                      # Third-party single-header libraries (stb_image)
│   ├── platform/                 # Native OS windowing & input headers (Windows/Linux/macOS)
│   └── utils/                    # Common system utilities
├── libs/                         # Modular Sub-Libraries & SDKs
│   ├── NetClient/                # Lightweight WinHTTP networking client library
│   └── omorekushe_core/          # Ultra-high performance Rust & SIMD acceleration core
├── packaging/                    # Distribution packaging configurations
│   └── windows/                  # Inno Setup installer script (OmorEkushe.iss)
├── src/                          # Application Core Source Code
│   ├── app/                      # Main Entry & Lifecycle (main.cpp, application.cpp)
│   ├── core/                     # Core Engines & Logic (phonetic engine, parser, hooks)
│   ├── error/                    # Win32/POSIX minidump crash handler
│   ├── platform/                 # Native OS implementations (Win32 Acrylic UI, Linux, macOS)
│   └── utils/                    # System utilities
├── tests/                        # Automated Unit & Regression Test Suite
│   ├── CMakeLists.txt            # Dedicated CTest build definition
│   └── test_phonetic_parser.cpp  # Avro phonetic grammar & transliteration tests (410+ assertions)
├── tools/                        # Companion Developer & Desktop Tools
│   ├── LayoutEditor/             # Dear ImGui visual keyboard layout designer
│   └── SpellingChecker/          # Bengali dictionary verification utility
├── .gitignore                    # Clean standard ignore rules (build/ fully untracked)
└── CMakeLists.txt                # Unified Root Modern CMake Configuration File
```

---

## Building from Source

### Prerequisites

- **Operating System**: Windows 10 / 11 (64-bit), Ubuntu Linux 22.04+, or macOS 12+
- **Compiler**: Visual Studio 2019 / 2022 (MSVC Desktop C++), MinGW-w64 GCC 9.0+, or Clang
- **Build System**: [CMake 3.16+](https://cmake.org/) & [Ninja](https://ninja-build.org/) (optional, recommended)

> [!NOTE]
> **No local installer tools required!** The developer machine only builds the pure C++ application binaries. The setup installer is automatically generated in the cloud by GitHub Actions CI.

### Visual Studio / Ninja Command

```powershell
# 1. Clone the repository
git clone https://github.com/sabbir28/OmorEkushe.git
cd OmorEkushe

# 2. Configure build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build all targets
cmake --build build --config Release

# 4. Run automated test suite via CTest (410+ tests)
ctest --test-dir build -C Release --output-on-failure
```

The compiled binaries are generated in `build/bin/OmorEkushe.exe` (with runtime assets dynamically deployed from `data/`).

---

## Installation & Distribution

### 1. Official Windows Setup Installer (`OmorEkushe-Setup.exe`)
- Download from [GitHub Releases](https://github.com/sabbir28/OmorEkushe/releases/latest).
- Built automatically by GitHub Actions using **Inno Setup**.
- Features:
  - Automatically registers Start Menu shortcuts and Desktop icon.
  - Automatically installs XML layout maps to `Layouts/`.
  - Upgrades seamlessly by automatically terminating running instances and updating files in place.
  - Silent installation / unattended upgrades:
    ```powershell
    OmorEkushe-Setup.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART
    ```

### 2. Standalone Portable Edition (`OmorEkushe-Portable.zip`)
- Download `OmorEkushe-Portable.zip`.
- Extract anywhere (e.g. `C:\Tools\OmorEkushe` or a USB drive) and launch `OmorEkushe.exe`. Zero administrative permissions required.

---

## Configuration & Settings

Omor Ekushe provides a rich native Win32 **Options Dialog** (`options_dialog.cpp`):

1. **Overlay Opacity**: Adjust transparency from 20% to 100% in real-time.
2. **Auto-Start with Windows**: Enable or disable launching on user login via Windows registry (`HKCU\Software\Microsoft\Windows\CurrentVersion\Run\OmorEkushe`).
3. **Custom Global Hotkeys**:
   - Layout Switching: Configurable modifiers (<kbd>Ctrl</kbd>, <kbd>Alt</kbd>, <kbd>Shift</kbd>) and key.
   - Phonetic Mode Shortcut: Configurable hotkey (default: <kbd>F12</kbd> or <kbd>Ctrl+Alt+P</kbd>).

To restore factory defaults, run:
```bat
reg delete "HKCU\Software\OmorEkushe" /f
```

---

## Visionary Future Roadmap

We are constantly evolving Omor Ekushe to make it the fastest, smartest Bengali typing platform in the world. For the detailed architectural specification, feasibility matrix, and full feature proposals, see [FUTURE_FEATURES.md](FUTURE_FEATURES.md).

Upcoming architectural milestones:

- [ ] **AI-Powered Predictive Suggestion Engine**:
  - Contextual n-gram and lightweight local LLM/Trie autocomplete displaying a discreet, non-intrusive floating suggestion pill for high-speed word completion.
- [ ] **Cloud Layout Marketplace & Hub**:
  - Explore, preview, and download community-created XML keyboard layouts directly through the UI via `NetClient`.
- [ ] **Background Silent Auto-Updater Daemon**:
  - Discreet background update checker that downloads latest releases from GitHub and applies Inno Setup `/VERYSILENT` upgrades seamlessly.
- [ ] **Cross-Platform Linux & macOS Core**:
  - Porting the core engine to Linux (using native X11 / Wayland input method protocol) and macOS (Input Method Kit).
- [ ] **Real-Time Contextual Bengali Spellchecking**:
  - Enhancing `SpellingChecker` with red-underline suggestions and phonetic distance autocorrection across any native text field.
- [ ] **Bengali Voice-to-Text Speech Typing**:
  - Integrating lightweight on-device speech recognition to type Bengali via microphone input.
- [ ] **Mobile Companion & Clipboard Sync**:
  - Sync custom dictionaries and clipboard history securely between Android/iOS and Windows desktop.

---

## Contributing

We welcome contributions from the global Bengali and open-source software community!

1. Fork the repository.
2. Create your feature branch (`git checkout -b feature/SmartPredictions`).
3. Commit your changes (`git commit -m 'feat: Add n-gram prediction trie'`).
4. Push to the branch (`git push origin feature/SmartPredictions`).
5. Open a Pull Request.

---

## License & Acknowledgments

- Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for details.
- Dedicated to the memory of the Language Movement Martyrs of February 21, 1952 (**মহান ভাষা শহিদদের স্মৃতির প্রতি গভীর শ্রদ্ধাঞ্জলি**).
- Developed and maintained by **Sabbir** and the **Omor Ekushe Open Source Community**.

