# ADR-0334: Keep frozen Qt DLL layout fallback consistent with plugin discovery

## Status

Accepted with limits — D298 / ARCH-268.

## Context

PyInstaller's public PyQt6 runtime hook selects the `PyQt6/Qt6` layout first
and falls back to the older `PyQt6/Qt` layout. QuillForge already used that
choice for plugin and `qwindows.dll` discovery, but the frozen Qt DLL preflight
introduced in D297 inspected only `PyQt6/Qt6/bin`. A valid legacy-layout bundle
could therefore be rejected before QApplication construction.

## Decision

`_startup_qt_runtime_dependencies()` now evaluates the same two layout roots in
the same order: `PyQt6/Qt6/bin` first, then `PyQt6/Qt/bin`. It returns success
as soon as one complete candidate contains QtCore, QtGui, QtWidgets, and
QScintilla. If neither candidate is complete, it reports the candidate with
the fewest missing files, preserving the existing `status`/`required`/`missing`
diagnostic shape. The D297 preflight and startup diagnostic therefore share one
layout-compatible inventory.

The targeted audit requires both roots, the ordered loop, and the complete-
candidate short circuit. No packaging contents or normal application policy
changes.

## Consequences

- Valid current and legacy PyQt6 frozen layouts are treated consistently.
- The current Qt6 candidate's required paths and successful report remain
  unchanged.
- An incomplete bundle still fails early through D297 with actionable paths.
- Native EXE startup, clean-machine behavior, and real Windows DLL loading
  remain unrun under the active `software_start_allowed=false` policy.

## Public-source applicability

This is Python 3.12/PyQt6/PyInstaller desktop packaging code. The PyInstaller
runtime hook in the installed toolchain is an engineering reference for this
local build; no manufacturer requirement, embedded C/C++, MCU, BSP/HAL, RTOS,
MISRA, ISO 26262, ASPICE, certification, or private ByteDance-standard claim
applies.

## Verification boundary

Compile, Ruff, format, presentation-contract audit, source startup/file-open
diagnostics, current-layout dependency reports, package identity, PE header,
and archive checks are required. A physical legacy-layout bundle and native
startup remain unrun checks.
