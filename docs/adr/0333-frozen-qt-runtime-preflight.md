# ADR-0333: Fail fast on an incomplete frozen Qt runtime

## Status

Accepted with limits — D297 / ARCH-267.

## Context

The portable candidate is a PyInstaller one-file Windows GUI build. A missing
Qt DLL, QScintilla extension, or `qwindows.dll` can make a windowed process
terminate before a Qt window exists. The existing no-window startup diagnostic
already reports these bundle dependencies, but the normal GUI path previously
went directly from argument parsing to `QApplication`, leaving the user with
no actionable message when the extracted bundle was incomplete.

## Decision

After the explicit plugin-host and diagnostic branches return, and immediately
before the normal `QApplication` import, `app.main()` calls the frozen-only
`_validate_frozen_qt_runtime()` helper. Source execution is a no-op. Frozen
execution reuses `_startup_qt_plugin_path()` and
`_startup_qt_runtime_dependencies()` and raises a `RuntimeError` containing
the missing bundle-relative paths when the platform plugin or required Qt/
QScintilla binaries are absent.

The existing `__main__` boundary catches that exception and retains the
localized native/stderr startup fallback and startup-error log. The static
presentation audit guards the branch ordering, pre-QApplication placement,
and reuse of the existing checks.

## Consequences

- An incomplete frozen bundle fails before Qt construction with an actionable
  error instead of an unexplained windowless exit.
- `--plugin-host`, all `--diagnose-*` routes, and source execution retain their
  existing dispatch and environment behavior.
- The change does not add a second dependency inventory, alter packaging
  contents, or change application composition/theme/document policy.
- Native EXE startup, clean-machine behavior, and actual Qt loader behavior
  remain unrun under the active `software_start_allowed=false` policy.

## Public-source applicability

This is Python 3.12/PyQt6/PyInstaller desktop code. No embedded C/C++, MCU,
BSP/HAL, RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE,
certification, or private ByteDance-standard claim applies.

## Verification boundary

Compile, Ruff, format, presentation-contract audit, source startup/file-open
diagnostics, frozen-helper simulation without Qt launch, package identity, PE
header, and archive checks are required. Direct frozen-process and clean-machine
startup remain explicit unrun checks.
