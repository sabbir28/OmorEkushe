# OmorEkushe LayoutEditor

`LayoutEditor` is a standalone Win32 GUI application for OmorEkushe, used to view, edit, and create keyboard layouts in an XML format. It visually maps standard keyboard keys to user-defined characters across various states (e.g., normal, Shift, Alt).

## Features

- **Visual Layout Editing**: Provides an interactive preview of a standard 104-key US keyboard. Clicking an editable key invokes a properties dialog.
- **Support for Modifiers**: Keys can be configured with output mapping for `Normal`, `Shift`, `Normal_Option`, and `Shift_Option` states.
- **XML Data Synchronization**: Load `.xml` files from standard layout paths (`Layouts\`, `..\data\layout\`, `..\data\Layouts\`), modifying them in-memory, and flush back to XML upon saving.
- **Juk (`<Juk>`) Parsing**: The core layout model structure natively parses and stores "Juk" combinations (sequence string mapped to an output string), although primarily edited in layout xmls.
- **Win32 Native UI**: Lightweight architecture built strictly against `user32` and `gdi32` components, eliminating third-party UI framework dependencies.

## Architecture & Components

The editor is composed of four main source components in `src/` and `include/`:

1. **`MainWindow.cpp` / `.h`**
   - Implements the main window host container `LayoutEditorWindow`.
   - Manages top-level layout states, drop-down selection of registered XMLs, and the "New Layout" / "Save" controls.
   - Coordinates file selection (e.g. browsing for an `.ico` layout icon).

2. **`KeyboardView.cpp` / `.h`**
   - Custom Win32 control `KeyboardViewControl` hosting a drawn representation of a standard keyboard layout.
   - Maps screen-relative percentages of width/heights dynamically to the parent's layout structure.
   - Draws normal/shifted character overrides dynamically reflecting `LayoutData`.
   - Propagates pointer interactions (clicks) to the main form to trigger the key editing interface.

3. **`KeyEditDialog.cpp` / `.h`**
   - Houses a pseudo-modal popup to mutate individual bindings.
   - Provides a fast, Win32 `STATIC` and `EDIT` layout for four main properties: `Normal`, `Shift`, `Normal Opt`, and `Shift Opt`.
   - Maps form updates back up to the passed underlying `LayoutData` structure.

4. **`LayoutData.cpp` / `.h`**
   - The data model and persistance layer.
   - Performs direct XML file stream (`_wfopen(filePath, L"r, ccs=UTF-8")`) and raw string tokenization to avoid heavy XML parsers.
   - Unescapes XML entities (`&amp;`, `&lt;`, etc.), reads layout properties, mapping sets (`<Key>`), and `<Juk>` structures.
   - Responsible for recursively probing disk structures via `FindLayouts()` for auto-discovery of `.xml` data inputs.

## Compilation

The `LayoutEditor` component is integrated as part of the overall CMake build pipeline of the OmorEkushe project.

By default, building the LayoutEditor is toggled via the CMake option:
```cmake
option(BIJOY_BUILD_LAYOUT_EDITOR "Build the LayoutEditor Win32 prototype" ON)
```

Target properties define Unicode declarations `UNICODE / _UNICODE` and link against necessary subsystems like `comctl32` and `shlwapi`. During integration, OmorEkushe's main UI provides a dedicated button or layout editor access point that attempts to execute `LayoutEditor.exe` from relative application paths.

## Usage Data Layout
An example layout XML format produced or parsed by this editor:
```xml
<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<!--Start of Layout-->
<KeyLayout>
  <Name>MyLayout</Name>
  <Shortcut>
    <Alt>False</Alt>
    <Ctrl>True</Ctrl>
    <Shift>False</Shift>
    <KeyCode>65</KeyCode>
  </Shortcut>
  <IconName>MyLayout.ico</IconName>
  <Keys>
    <Key KeyCode="81" Normal_Option="q" Shift_Option="Q">
      <Normal>q_repl</Normal>
      <Shift>Q_REPL</Shift>
    </Key>
  </Keys>
  <Juks>
    <Juk>
      <Seq>abc</Seq>
      <Out>xyz</Out>
    </Juk>
  </Juks>
</KeyLayout>
<!--End of Layout-->
```
