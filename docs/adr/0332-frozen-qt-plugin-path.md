# ADR-0332: Bind frozen Qt plugin discovery to the application bundle

## Status

Accepted with limits — D296 / ARCH-266.

## Context

The portable candidate is a PyInstaller one-file Windows GUI build. PyInstaller's
PyQt6 runtime hook normally configures `QT_PLUGIN_PATH`, but a packaged process
can still inherit or observe a conflicting Qt platform-plugin environment before
`QApplication` is constructed. A missing or foreign `qwindows.dll` path presents
to the user as an EXE that opens and immediately disappears or never shows a
window.

## Decision

`quillforge.app.main()` calls one small `_configure_frozen_qt_plugins()` helper
before any `QApplication` import. The helper is a no-op for source execution and
for bundles without a discoverable plugin directory. For a frozen bundle it
selects the current PyQt6 Qt6/Qt plugin directory (with the legacy Qt fallback)
and binds both `QT_PLUGIN_PATH` and the platform-specific plugin path to that
bundle. The existing PyInstaller runtime hook remains in place; the application
boundary provides a final, explicit bundle-local contract.

The presentation contract audit guards the frozen-only branch, both supported
PyQt layouts, both environment assignments, and ordering before QApplication
import.

## Consequences

- The portable candidate does not depend on a developer or machine-level Qt
  plugin path when the bundle contains its own plugins.
- Source execution and Qt-free diagnostics retain their existing behavior.
- The helper does not install a new dependency, alter theme/application policy,
  or move plugin ownership into composition/presentation modules.
- Native EXE startup, clean-machine behavior, and platform-plugin loading remain
  unrun under the active `software_start_allowed=false` policy.

## Public-source applicability

This is Python/PyInstaller/PyQt6 desktop packaging code. No embedded C/C++,
MCU, BSP/HAL, RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE,
certification, or private ByteDance-standard claim applies.

## Verification boundary

Compile, Ruff, format, presentation-contract audit, source startup/file-open
diagnostics, package identity, PE header, and frozen archive checks are required.
The direct frozen-process and clean-machine startup checks remain explicit
limits and must not be inferred from static archive evidence.
