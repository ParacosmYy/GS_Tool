# ADR-0092: MainWindow file-dialog/About forwarding simplification

- **Status:** accepted-with-limits; D67 / ARCH-51 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

After D65, `MainWindow` still had two private methods with no coordinator
policy: `_choose_save_path()` returned the existing `FileDialogSurface` result,
and `_show_about()` returned the existing `MessageSurface` call. The aliases
made the coordinator look like the owner of presentation queries without
adding a semantic contract.

## Decision

Delete both aliases and call the existing presentation surfaces at their
callers:

- Save As and dirty-tab close call
  `FileDialogSurface.choose_save_path(...)` directly;
- the `help.about` command binds directly to
  `MessageSurface.show_about`.

`FileDialogSurface` remains responsible for native file-dialog composition and
locale; `MessageSurface` remains responsible for About composition and locale.
MainWindow retains all save, close, command-registry, dirty-state, recovery,
and application policy.

## Invariants

1. Save As uses the current path as the native dialog's default name.
2. Dirty-tab close still chooses Save/Discard/Cancel before any path dialog,
   and a selected path still enters the existing `_start_save` callback path.
3. Command ID `help.about`, title, menu placement, and execution timing remain
   unchanged; only the callable target loses one forwarding layer.
4. No FileDialogSurface, MessageSurface, MainWindow policy, locale, service,
   persistence, or callback contract is moved or broadened.
5. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active policy.

## Alternatives considered

- **Keep the aliases for local readability:** rejected; neither method names a
  policy or transforms its arguments.
- **Create a DialogCoordinator:** rejected; it would add an abstraction around
  two already cohesive presentation surfaces.
- **Move save/close policy into FileDialogSurface:** rejected; the surface
  must remain a path-picker, while MainWindow owns document policy.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-coordinator code.
Embedded C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power, motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Verification target and limits

- Source probe confirms both aliases are absent and direct call targets are
  retained at Save As, dirty-close, and `help.about` command sites.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D67 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native callback timing, runtime startup, clean-machine, cross-machine,
  accessibility, and external release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
