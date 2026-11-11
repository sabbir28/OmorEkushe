# Omor Ekushe: Next-Generation Feature Proposals and Architectural Roadmap

> **Document Version**: 2.0.0  
> **Target Timeline**: 2026 - 2027 Release Cycles  
> **Status**: Approved RFC & Development Specification  

---

## Executive Summary

Omor Ekushe has evolved into a high-performance, native Bengali input ecosystem backed by C++17, Rust, and x86_64 SIMD/Assembly acceleration. To build upon this foundation and maintain leadership as the most reliable, modern, and memory-efficient Bengali computing platform in the world, this document outlines the proposed feature catalog for upcoming releases.

The features are categorized into four strategic pillars:
1. **Linguistic Intelligence and Typing Engine Acceleration**
2. **System Architecture and OS Protocol Modernization**
3. **Cross-Platform and Ecosystem Tooling**
4. **Multimodal and Accessibility Technologies**

---

## 1. Linguistic Intelligence and Typing Engine

### 1.1 Predictive Next-Word Suggestion Engine
- **Overview**: Provide real-time next-word autocomplete and candidate suggestions via a non-intrusive, floating candidate window anchored to the active text cursor.
- **Technical Specification**:
  - Implement a weighted Trie with a compressed n-gram language model (Uni-gram and Bi-gram frequencies derived from modern Bengali literature and colloquial corpora).
  - Use SIMD-accelerated prefix search in `libs/omorekushe_core`.
  - Keyboard interactions: Press <kbd>Tab</kbd> or number keys (<kbd>1</kbd>-<kbd>5</kbd>) to commit candidate suggestions without breaking typing rhythm.
  - Expected Latency: Under 1 millisecond candidate generation per keystroke.

### 1.2 Contextual Auto-Correction and Disambiguation
- **Overview**: Automatically resolve common spelling ambiguities in real time based on surrounding word context.
- **Technical Specification**:
  - Disambiguation rules for homophonous characters:
    - হ্রস্ব ইকার (`ি`) versus দীর্ঘ ঈকার (`ী`)
    - হ্রস্ব উকার (`ু`) versus দীর্ঘ ঊকার (`ূ`)
    - মূর্ধন্য ণ (`ণ`) versus দন্ত্য ন (`ন`)
    - শ (`শ`), ষ (`ষ`), স (`স`)
  - Integration with the Rust `spelling::myers_levenshtein_64` engine to detect transposition errors and offer single-click corrections.

### 1.3 Memory-Mapped Binary Dictionary Format (.cdb / FlatBuffers)
- **Overview**: Eliminate raw plain-text parsing of large dictionary files on application startup.
- **Technical Specification**:
  - Replace `bangla_wordlist.txt` with a pre-compiled, static binary hash format (`.bin` / `.cdb`) mapped directly into virtual memory via `CreateFileMapping` / `mmap`.
  - Cold startup time reduced from ~120ms to under 2ms.
  - Resident memory footprint (RAM) reduced below 4 MB, as unused dictionary pages remain unpaged by the OS kernel.

### 1.4 Smart Text Expander and Date/Currency Macro Engine
- **Overview**: Power-user productivity shortcuts that automatically expand predefined trigger prefixes into formatted Bengali text.
- **Examples**:
  - Trigger `//date` or `//tarikh` &rarr; `১১ নভেম্বর ২০২৬`
  - Trigger `//time` &rarr; `১১:৪২ অপরাহ্ন`
  - Number to Words conversion: `1500/=` &rarr; `এক হাজার পাঁচ শত টাকা মাত্র`
  - Custom user snippets configurable via a simple key-value settings interface.

---

## 2. System Architecture and OS Integration

### 2.1 Native Windows Text Services Framework (TSF) TIP Provider
- **Overview**: Transition from legacy low-level keyboard hooks (`WH_KEYBOARD_LL`) to a native Windows Text Services Framework (TSF) Input Processor.
- **Technical Specification**:
  - Eliminates compatibility issues in sandboxed environments (Windows Sandbox, UWP, modern Chromium / Electron apps, and Microsoft Office).
  - Provides direct inline composition strings (underlined active composition before committing), matching the native Japanese and Chinese IME experience in Windows 11.
  - Fixes application-level focus stealing and caret positioning across multi-monitor setups.

### 2.2 Fluent 2.0 Mica / Acrylic Desktop Ribbon
- **Overview**: Overhaul the overlay floating toolbar with modern Windows 11 Fluent Design System aesthetics.
- **Technical Specification**:
  - Hardware-accelerated DirectComposition / Direct2D rendering.
  - Dynamic Mica and Acrylic translucent background materials with light and dark mode synchronization (`DwmSetWindowAttribute`).
  - Smooth DPI scaling transitions across mixed-DPI multi-monitor workstations.
  - Compact mini-pill mode that auto-collapses to a micro indicator after inactivity.

### 2.3 Lock-Free Ring Buffer Keystroke Dispatcher
- **Overview**: Guarantee zero dropped keystrokes and zero input lag, even under heavy CPU loads or gaming scenarios.
- **Technical Specification**:
  - Implement a Single-Producer Single-Consumer (SPSC) lock-free ring buffer between the Windows hook/TSF callback and the transliteration engine worker thread.
  - Keystroke dispatch thread pinned to high priority (`THREAD_PRIORITY_TIME_CRITICAL`).

### 2.4 Background Silent Auto-Updater Daemon
- **Overview**: Automatic update checks and delta installations without user interruption.
- **Technical Specification**:
  - Light daemon service or scheduled task checking the official GitHub Releases API via `libs/NetClient`.
  - Validates SHA-256 binary signatures.
  - Executes Inno Setup in `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART` mode when the system is idle or during application restart.

---

## 3. Cross-Platform and Tooling Ecosystem

### 3.1 Linux Desktop Engine (IBus / Fcitx5 / Wayland)
- **Overview**: Bring Omor Ekushe to the Linux desktop with native input method integrations.
- **Technical Specification**:
  - Package `libs/omorekushe_core` as a shared C library (`libomorekushe_core.so`).
  - Implement an IBus engine (`ibus-omorekushe`) and Fcitx5 addon.
  - Full Wayland text-input protocol support without X11 emulation dependencies.

### 3.2 macOS Input Method Kit (IMK) Bundle
- **Overview**: Native macOS input source integration.
- **Technical Specification**:
  - Swift / Objective-C bridge linking directly to `omorekushe_core` static archive.
  - Installs as an official Input Method in System Settings &rarr; Keyboard &rarr; Input Sources.

### 3.3 Visual Drag-and-Drop Keyboard Layout Designer
- **Overview**: Upgrade `tools/LayoutEditor` into an intuitive visual layout creation suite.
- **Technical Specification**:
  - Interactive on-screen visual keyboard representing standard 104-key and laptop layouts.
  - Drag-and-drop character assignment for Normal, Shift, AltGr, and Shift+AltGr states.
  - Instant live test canvas with syntax validation for conjuncts (`হসন্ত` sequences).
  - Export and import in standard JSON and legacy XML formats.

### 3.4 Community Layout and Dictionary Cloud Hub
- **Overview**: Allow users to browse, download, and share specialized keyboard layouts and custom dictionaries.
- **Technical Specification**:
  - Built-in browser in `LayoutEditor` powered by `NetClient`.
  - One-click installation of community layouts (e.g., Munir, Prabhat, Jatiya, Unijoy, Customized Phonetic variations).

---

## 4. Multimodal and Accessibility Technologies

### 4.1 On-Device Offline Bengali Voice-to-Text Typing
- **Overview**: Enable voice typing by converting speech directly to Bengali text locally on the machine.
- **Technical Specification**:
  - Embed quantized ONNX runtime running a compact Bengali acoustic model (e.g., Whisper-Tiny or fine-tuned Kaldi model).
  - 100% offline and private: zero audio data transmitted to external servers.
  - Push-to-talk shortcut (e.g., <kbd>Win</kbd> + <kbd>H</kbd> or <kbd>Ctrl</kbd> + <kbd>Space</kbd>).

### 4.2 Interactive On-Screen Touch Keyboard
- **Overview**: Full touch-screen support for Windows tablets (Surface Pro, 2-in-1 laptops).
- **Technical Specification**:
  - Multi-touch capable UI rendered with Direct2D.
  - Visual phoneme grouping with automatic kar and yuktakshar suggestion popups on key hold.

### 4.3 Screen Reader (NVDA / Narrator) Accessibility
- **Overview**: First-class support for visually impaired Bengali typists.
- **Technical Specification**:
  - Implement Windows UI Automation (UIA) provider interfaces across all windows and overlays.
  - Audio feedback cues on phonetic mode toggle and layout cycle.
  - Character and word echo integration for NVDA and Windows Narrator.

### 4.4 Encrypted Cross-Device Dictionary Sync
- **Overview**: Seamlessly sync personal dictionaries and custom wordlists across all user devices.
- **Technical Specification**:
  - End-to-end encrypted backup using AES-256-GCM.
  - Storage backends: Private GitHub Gist or user-specified WebDAV / cloud drive folder.

---

## 5. Implementation Priority Matrix

| Feature | Pillar | Complexity | Target Release | Dependencies |
| :--- | :--- | :--- | :--- | :--- |
| **Predictive Next-Word Engine** | Linguistic | Medium | Completed (v1.2.0) | Trie + Rust Core |
| **Smart Text Expander & Macros** | Linguistic | Low | Completed (v1.2.0) | Rust + C/C++ Engine |
| **Memory-Mapped Dictionary (.cdb)**| Performance | Medium | v1.2.0 | OS File Mapping |
| **Fluent 2.0 Ribbon & Mica UI** | Architecture | Medium | v1.2.0 | Direct2D / DWM |
| **Lock-Free Ring Buffer Hooks** | Architecture | Medium | v1.3.0 | Atomic C++17 |
| **Silent Auto-Updater Daemon** | Architecture | Low | v1.3.0 | NetClient + Inno |
| **Windows TSF Native TIP Provider**| Architecture | High | v1.4.0 | COM / Windows TSF |
| **Visual Drag-and-Drop Layout Editor**| Tooling | Medium | v1.4.0 | ImGui / Layout Parser |
| **Linux IBus / Wayland Port** | Cross-Platform | High | v2.0.0 | IBus / Wayland |
| **macOS IMK Bundle** | Cross-Platform | High | v2.0.0 | macOS SDK |
| **On-Device Voice Typing** | Multimodal | Very High | v2.1.0 | ONNX Runtime |
| **Screen Reader UIA Integration** | Accessibility | Medium | v2.1.0 | UI Automation API |

---

## 6. RFC and Contribution Workflow

Developers wishing to implement any of the features listed above should follow these guidelines:

1. **Open an Issue / RFC**: Create a design proposal under GitHub Issues referencing the feature title from this document.
2. **Benchmark Requirement**: Any changes touching `libs/omorekushe_core` or `src/phonetic/` must include a criterion benchmark verifying zero performance regression.
3. **Coding Standards**:
   - Modern C++17 for application and UI layers.
   - Safe Rust with SIMD vectorization for performance-critical engines.
   - Zero compiler warnings (`/W4 /WX` on MSVC, `-Wall -Wextra -Werror` on GCC/Clang).
   - Strict adherence to the clean architecture and zero-emoji policy across all code and documentation.

