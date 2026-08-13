# ADR-0335: Prefer a frozen Qt plugin root that contains qwindows.dll

## Status

Accepted with limits — D299 / ARCH-269.

## Context

D296 supported both `PyQt6/Qt6/plugins` and the legacy
`PyQt6/Qt/plugins` layout, but selecting the first existing directory could
mask a usable fallback. A bundle may contain the current Qt6 plugin directory
without its `platforms/qwindows.dll` while the legacy directory is complete.
That split-brain selection can make a portable GUI fail before the normal
startup error boundary has useful context.

## Decision

`_frozen_qt_plugin_root()` is the single selector for frozen plugin discovery
and startup diagnostics. It checks the current Qt6 root first and the legacy
Qt root second, but prefers the first candidate that contains
`platforms/qwindows.dll`. If neither candidate has that platform plugin, it
retains the first existing directory (or the Qt6 path as the deterministic
fallback) so the existing preflight can report the missing bundle path.

`_configure_frozen_qt_plugins()` and `_startup_qt_plugin_path()` both reuse
this selector. Source execution remains a no-op, the frozen-only boundary and
environment-variable ownership remain unchanged, and no application,
document, theme, plugin-policy, or user-data ownership moves.

The presentation contract audit now guards the shared selector and its
`qwindows.dll` preference.

## Consequences

- A complete legacy plugin root is selected when an incomplete Qt6 directory
  would otherwise mask it.
- Normal Qt6 bundles continue to select the Qt6 root.
- Incomplete bundles retain deterministic, actionable failure reporting.
- Native EXE/Qt startup, clean-machine behavior, and real Windows DLL loading
  remain unrun under the active `software_start_allowed=false` policy.

## Public-source applicability

This is Python 3.12/PyQt6/PyInstaller desktop packaging code. The installed
PyInstaller runtime hook is an engineering reference for this local build;
no manufacturer requirement, embedded C/C++, MCU, BSP/HAL, RTOS, MISRA,
ISO 26262, ASPICE, certification, or private ByteDance-standard claim
applies.

## Verification boundary

Compile, Ruff, format, presentation-contract audit, source startup/file-open
diagnostics, selector simulation, package identity, PE header, and archive
checks are required. A physical split-layout bundle, native startup,
clean-machine run, and real Windows DLL-loader behavior remain unrun checks.
