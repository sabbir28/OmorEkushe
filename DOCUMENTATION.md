# Omor Ekushe C++ – Technical Documentation

Detailed notes for developers: design, modules, and implementation details.

---

## 1. Architecture

- **Single process, one main thread:** WinMain creates the main window (hidden), shows the registration dialog modally, then runs the message loop. The low-level keyboard hook runs in the same process and is called on the thread that installed it (or the target thread); key handling is synchronous.
- **No .NET:** All UI is Win32 (dialogs, tray icon, menus). Registry via `RegOpenKeyEx` / `RegQueryValueEx` / `RegSetValueEx`. File I/O via `_wfopen` and C runtime.

---

## 2. Module Details

### 2.1 main.cpp

- Calls `InitCommonControlsEx` for common controls.
- `GetAppDirectory()` + `FindLayouts(g_layouts, appDir)` to load layouts before UI.
- If no layouts found, shows an error and exits.
- `KeyboardHook_Install(hInstance)` then `KeyboardHook_SetLayoutsReady(true)`.
- Creates main window (hidden), shows registration dialog, then shows main window and runs message loop.
- On exit, `KeyboardHook_Uninstall()`.

### 2.2 native_api

- **Structures:** Match Win32 `INPUT`, `KEYBDINPUT`, `MOUSEINPUT`, `HARDWAREINPUT` (packed) for `SendInput`.
- **Enums:** `InputType`, `KEYEVENTF` (EXTENDEDKEY, KEYUP, UNICODE, SCANCODE).
- **Functions:**
  - `SendInput(nInputs, pInputs, cbSize)` – thin wrapper around `::SendInput`.
  - `DoKeyBoard(flags, scanCode)` – one KEYBDINPUT with `wVk=0`, `wScan=scanCode`, `dwFlags=flags` (used for Unicode input).
  - `DoKeyBoardVk(flags, vk)` – one KEYBDINPUT with virtual key.

### 2.3 keyboard_hook

- **Hook type:** `WH_KEYBOARD_LL` (low-level keyboard).
- **Logic (simplified):**
  - If Ctrl/Alt/Shift + key matches a layout shortcut: toggle that layout for the foreground window (add/remove in handle_info, update app_state). Consume key (return 1).
  - If no modifier and a layout is current: look up `vkCode` in that layout’s `key` map; if found, output Normal or Shift string via `DoKeyBoard(UNICODE)` and consume key.
- **Not implemented in C++:** Juk (composition) sequences, Delay/DeInc/MoveL/Xtra options, backspace/enter queue, and per-window layout recall on focus (handle_info is updated but not yet read when switching windows).

### 2.4 layout

- **Layout:** name, iconName, path, id, shortcut (alt/ctrl/shift/keyCode), `map<int, KeyMapping>` key, `map<wstring, wstring>` juk.
- **KeyMapping:** normal, shift, normalOption, shiftOption (options like Delay/MoveL not yet used in C++).
- **XML:** Simple line-based parser; looks for `<Name>`, `<Shortcut>` (Alt/Ctrl/Shift/KeyCode), `<IconName>`, `<Key KeyCode="..." Normal_Option="..." Shift_Option="...">` with `<Normal>`/`<Shift>`, and `<Juk>` with `<Seq>`/`<Out>`. UTF-8 file read via `_wfopen(..., L"r, ccs=UTF-8")`.

### 2.5 handle_info

- Static `vector<HandleInfo>` (hwnd + Layout*). `AddInfo` removes any existing entry for the hwnd, pushes new one, then prunes invalid windows. `FindInfo` returns layout for hwnd. `RemoveInfo` removes entry for hwnd.

### 2.6 app_state

- Global: `g_layouts`, `g_layoutCount`, `g_currentLayoutIndex`, `g_comLayoutSelectedIndex`.
- `AppState_SetCurrentLayout(index)` (use -1 for “English”/no layout).
- Getters: `AppState_GetCurrentLayoutIndex`, `AppState_GetCurrentLayout`, `AppState_GetLayoutByIndex`, `AppState_GetLayoutCount`.

### 2.7 my_functions

- **Password:** Static `Password` string; `GetRegistryPassword(out)`, `SetRegistryPassword(value)` for `SOFTWARE\OmorEkushe\Options`.
- **GetMachineId:** `GetComputerNameW` (fallback "PC").
- **VerifyPath:** Ensures path ends with `\`.
- **Encrypt:** Simplified compared to C# (used for activation logic).
- **Drag:** `ReleaseCapture` + `SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0)` for window drag (not used in current C++ UI).

### 2.8 find_layouts

- **GetAppDirectory:** `GetModuleFileNameW` + `PathRemoveFileSpecW` + trailing `\`.
- **FindLayouts:** Recursive search under `appDir + Layouts\` for `*.xml`; for each file, creates a `Layout` and `loadFromFile`; fills the passed `vector<Layout>`.

### 2.9 dlg_registration

- Dialog proc: WM_INITDIALOG loads saved password from registry; if it matches, hides dialog and (if provided) shows splash, then EndDialog(1). Otherwise shows machine ID in read-only edit and waits for user.
- OK: Concatenates the four code edit fields; if length and value match `my_functions::Password`, writes registry, hides dialog, shows splash if provided, EndDialog(1).
- Cancel / Close: EndDialog(0).

### 2.10 dlg_main

- **Main window:** Created with a simple WNDCLASSEX and WinProc; used as owner for tray and message loop. Can be shown/hidden via tray “Show” and close minimizes to tray.
- **Tray:** `NOTIFYICONDATAW`, NIF_ICON | NIF_MESSAGE | NIF_TIP; callback message WM_TRAYICON. Right-click shows popup menu: Show, layout items (IDM_TRAY_LAYOUT_BASE + index), Exit. Double-click left: Show.
- **Menu:** Built at WM_CREATE from `AppState_GetLayoutCount()` and layout names; no dynamic refresh when layouts change.

---

## 3. Resource Files

- **resource.h:** IDI_APP, IDD_REGISTRATION, IDD_MAIN, IDC_* for registration controls, ID_TRAY, IDM_TRAY_*.
- **app.rc:** IDD_REGISTRATION dialog (labels, edit controls, OK/Cancel). Icon line commented out; uncomment and add `app.ico` for custom icon.

---

## 4. Threading and Hook

- The hook runs on the thread that receives the message (target thread). No extra worker threads. All state (app_state, handle_info, g_layouts) is accessed from that context; no locking in the current design. If you add background loading or UI updates from the hook, add proper synchronization.

---

## 5. Compatibility with C# Layout XML

- The C++ parser expects the same general KeyLayout XML shape (Name, Shortcut, IconName, Keys with Key elements, Juks with Juk elements). Attribute and tag names must match. Optional elements can be omitted. Encoding should be UTF-8 for correct Unicode.

---

## 6. Build Notes

- **Unicode:** Project uses `UNICODE`/`_UNICODE`; all Win32 calls are wide (`*W`).
- **Runtime:** No special DLLs; static or default CRT. ComCtl32 and Shell32 are linked for tray and common controls.
- **RC:** `app.rc` is compiled by the platform RC (MSVC or windres). Ensure `resource.h` is in the same directory as `app.rc` or in the include path.
