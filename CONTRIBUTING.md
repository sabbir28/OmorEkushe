# Contributing to Omor Ekushe

Thank you for your interest in contributing to **Omor Ekushe (অমর একুশে)**! We welcome contributions of all kinds, including bug reports, feature requests, code improvements, documentation updates, and new Bengali keyboard layout XML maps.

---

## How to Contribute

### 1. Reporting Bugs
Before opening a bug report, please check existing issues to ensure it hasn't been reported. When filing an issue:
- Use the **Bug Report** template.
- Describe the expected behavior vs actual behavior.
- Include your Windows version (e.g., Windows 10, Windows 11).
- Provide clear steps to reproduce the issue.

### 2. Requesting Features
- Use the **Feature Request** template.
- Explain clearly why this feature would be useful to Omor Ekushe users.
- Describe your proposed solution or ideas.

### 3. Contributing Keyboard Layouts
We welcome new Bengali keyboard layout XML maps! To contribute a layout:
- Create an XML file following the schema documented in `README.md`.
- Place the XML in `data/` or test it using the `LayoutEditor`.
- Verify key mappings across normal, Shift, and AltGr states.

### 4. Code Contributions
- Fork the repository and create your feature branch (`git checkout -b feature/my-feature`).
- Follow C++17 standards and maintain project formatting.
- Ensure your changes compile cleanly on MSVC (`/W4`) or MinGW (`-Wall -Wextra`).
- Open a Pull Request with a descriptive title and summary of changes.

---

## Code Style & Standards

- **Language Standard**: C++17
- **Formatting**: Clean indentations (4 spaces), explicit variable naming.
- **Naming Conventions**:
  - Classes & Structures: `PascalCase` (e.g., `KeyboardHookService`, `AppState`)
  - Functions & Methods: `snake_case` or `camelCase` (e.g., `initialize_hooks()`, `getLayout()`)
  - Member Variables: Prefix with `m_` (e.g., `m_hWnd`, `m_currentLayout`)
- **Character Encoding**: UTF-8 (Unicode `UNICODE` / `_UNICODE` defines enabled).

Thank you for helping preserve and enhance Bengali computing tools!
