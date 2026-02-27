# Replatforming Strategy: Legacy C# Omor Ekushe to Native C/C++

This document explains the migration strategy for moving the legacy C# Windows desktop application (last materially maintained around 2014) to a modern, maintainable native C/C++ codebase.

## Why this project exists

The original product is still operationally critical for many Bangla users in Bangladesh, but the upstream application is effectively abandoned. The legacy stack creates long-term risk:

- Aging runtime and UI dependencies.
- Difficult maintenance and onboarding for new contributors.
- Limited ability to improve low-level keyboard/IME behavior and performance.

## Core objective

Build a deterministic, testable native engine that preserves Bijoy-style typing behavior while removing dependence on the legacy .NET application stack.

## Migration approach

### 1) Recover behavior from the legacy binary

- Decompile and inspect the C# implementation.
- Extract business logic rather than reproducing UI forms one-to-one.
- Catalog feature parity requirements in migration checklists.

### 2) Isolate and formalize typing algorithms

Prioritize the logic that defines user-visible behavior:

- Key mapping rules (normal/shift/modifier handling).
- Encoding and Unicode output decisions.
- Shortcut-based layout switching.
- IME/text transformation semantics (including composition behavior where applicable).

### 3) Re-implement in native C/C++ modules

- Build clear, separable modules for layout parsing, keyboard hook processing, and output injection.
- Keep Win32 integration at the boundaries so the core logic stays portable and testable.
- Use explicit state transitions and predictable processing order to avoid legacy side effects.

### 4) Decouple engine from UI

- Treat registration, tray/menu, and dialogs as adapters.
- Keep typing logic independent from UI concerns.
- Make future frontends possible without rewriting the engine.

## Target architecture

The long-term architecture is intentionally layered:

1. **Core engine (C/C++)**
   - Stateless helpers + explicit state containers.
   - Layout/key transformation pipeline.
   - Deterministic input/output behavior.

2. **Platform adapter (Win32 today)**
   - Low-level keyboard hook.
   - Foreground window and per-window layout context.
   - Output injection (`SendInput`) and OS message integration.

3. **Presentation layer (current/future UIs)**
   - Tray and configuration dialogs (current).
   - Optional future shells/bindings (native UI variants, service wrappers, other host apps).

## Definition of success

A successful replatform preserves end-user typing behavior while improving engineering fundamentals:

- Maintainability: contributors can reason about modules in isolation.
- Performance: lower overhead and tighter OS-level control.
- Longevity: reduced dependency risk and easier future evolution.
- Extensibility: support additional UIs/integrations without touching core typing rules.

## Near-term implementation priorities

- Close known parity gaps listed in `MISSING_FUNCTIONALITY.md`.
- Increase automated tests around transformation and shortcut behavior.
- Continue documenting recovered legacy semantics as executable rules.
- Gradually move from “feature port” to “engine-first” project structure.
