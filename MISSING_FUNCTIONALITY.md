# Missing Functionality – C++ Port vs C# Original

This document lists features and behaviors of the original C# Bijoy Bayanno application that are **not implemented** or are **reduced** in the current C++ port. Use it as a checklist for bringing the C++ version to parity.

---

## 1. UI and Forms

| Feature | C# | C++ | Notes |
|--------|----|-----|------|
| **Splash screen** | frmSplash: fade-in, timer, then load options and show frmMain | Smoll modification has been done | C++ goes straight from registration to main/tray. No splash window. |
| **Main form (visible)** | Floating bar with combo (layout list), picIco, buttons (Options, Layout Editor, Minimize, Close), tooltips | Only tray + popup menu | No visible “main window” bar. No combo, no Options/Editor buttons. |
| **Fade in/out** | FadeTimer: opacity 0↔1, then hide/show tray icon | Not implemented | No opacity animation. |
| **Options dialog** | frmOptions: tabs (e.g. General, Layout Activation Mode), registry load/save, LAM checkbox | Not implemented | No settings UI. |
| **Layout Editor** | frmLayoutCreator: full layout editor (keys, Juks, save XML) | Not implemented | Layouts are read-only; no editor. |
| **Dictionary dialog** | frmDictionary: labels (e.g. English) | Not implemented | Not ported. |
| **frmDlg** | Expand/collapse, TextBox, OK/Cancel | Not implemented | Not ported. |
| **Tray menu icons** | Layout icons from Icons folder per layout | Default icon only | C++ uses one icon; no per-layout icons in menu. |
| **“English” in menu** | First item “English” (index 0), separator | Only layout names from XML | No explicit “English” entry; index 0 = first layout. |

---

## 2. Layout and Keyboard Logic

| Feature | C# | C++ | Notes |
|--------|----|-----|------|
| **Juk (composition)** | Que of last N chars; on key, check Juk map and replace sequence with output; backspace handling | Not implemented | Only simple key→output; no composition sequences. |
| **Delay / DeInc / MoveL / Xtra** | Special Normal_Option/Shift_Option handling (delay timer, backspace, arrow, extra char) | Not implemented | Options in XML are parsed but not used. |
| **Backspace / Enter in queue** | ReQue for backspace (♠) and Enter (\r\n) to support Juk and positioning | Not implemented | No input queue. |
| **Per-window layout** | CheckHandle timer: on focus change, look up HWND in mHandleInfo and set comLayouts + curLayout | Partially implemented | handle_info stores HWND→Layout; tray layout selection now calls AddInfo(foreground). Hook does not yet restore layout when focus changes (no CheckHandle timer). |
| **Default layout** | defLayout from registry; applied on startup and when switching to “English” | Not implemented | No registry load for DefaultLayout; no “default” layout concept. |
| **Registry shortcut overrides** | Layout Init reads shortcut from `BijoyEkushe\Options\<Name>` if KeyCode exists | Not implemented | Only XML shortcut used. |

---

## 3. Options and Persistence

| Feature | C# | C++ | Notes |
|--------|----|-----|------|
| **Registry keys (BijoyEkushe)** | ApplicationMode, Password, TrayMode, DefaultLayout, Position, LAM | Only BijoyBayanno\Options Password | Other options not saved/restored. |
| **ApplicationMode** | 1/2/3: normal, always visible, tray only | Not implemented | No mode; behavior is effectively tray-only after registration. |
| **TrayMode** | Start in tray vs start with main form visible | Not implemented | Main window is shown after registration; no “start in tray” option. |
| **Position** | Main form Left saved/restored | Not implemented | No main form position persistence. |
| **LAM (Layout Activation Mode)** | CheckHandle.Enabled: per-window layout on focus | Not implemented | No timer; per-window layout not applied on focus. |

---

## 4. Registration and Activation

| Feature | C# | C++ | Notes |
|--------|----|-----|------|
| **Machine ID** | WMI Win32_PhysicalMedia (serial etc.) | GetComputerNameW | C++ uses computer name only; easier to replicate across machines. |
| **Encrypt** | Full C# obfuscation (reverse, LCase, digit mix) | Simplified version | C++ encrypt is a placeholder; same password string works but algorithm differs. |
| **4-part code** | 5 edit fields (one + 4), concatenated and compared to Password | Same 5 fields, same logic | Implemented. |
| **rHide** | After valid login, next Activated hides registration form | Not needed | C++ uses modal dialog; no separate “activated” hide. |

---

## 5. Main Form Behavior (C# Only)

| Feature | C# | C++ |
|--------|----|-----|
| comLayouts selection → curLayout, NotifyIcon icon, picIco, defLayout | ✓ | Only in tray menu; no combo, no icon update. |
| NotifyMenu item click → comLayouts.SelectedIndex from Tag | ✓ | Tray menu sets layout by index. |
| OptionToolStripMenuItem / btnOptions → frmOptions.ShowDialog | ✓ | No Options dialog. |
| LayoutEditorToolStripMenuItem / btnLEditor → frmLayoutCreator | ✓ | No Layout Editor. |
| btnClose → fade then exit | ✓ | Exit from tray only. |
| btnMinimize → fade to tray | ✓ | Close minimizes to tray. |
| CheckHandle_Tick: prune invalid windows, get foreground, apply layout from handle_info | ✓ | Not implemented. |

---

## 6. Summary Checklist

- [done] Splash screen and startup sequence (fade, load options, then show main).
- [ ] Visible main form (floating bar with layout combo, icon, buttons).
- [ ] Options dialog and persistence (ApplicationMode, TrayMode, DefaultLayout, Position, LAM).
- [ ] Layout Editor (frmLayoutCreator).
- [ ] Dictionary and frmDlg if required.
- [ ] Juk (composition) and special options (Delay, DeInc, MoveL, Xtra).
- [ ] Per-window layout: timer to apply layout on focus change using handle_info.
- [ ] Default layout and registry shortcut overrides.
- [ ] Tray and main form icons from Icons folder.
- [ ] “English” as first menu item and proper index 0 handling.
- [ ] Stronger machine ID and Encrypt to match C# (optional for compatibility).

Implementing the items above would bring the C++ port much closer to the original C# application’s functionality.
