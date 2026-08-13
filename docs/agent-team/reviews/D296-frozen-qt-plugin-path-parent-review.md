# D296 parent review — frozen Qt plugin path

## Scope

Reviewed `src/quillforge/app.py`, the PyInstaller PyQt6 runtime-hook contract,
the one-file archive contents, the startup diagnostic path, and the targeted
presentation audit.

## Findings

- PASS — `_configure_frozen_qt_plugins()` runs before any `QApplication` import.
- PASS — source execution is a no-op; no source environment or diagnostic
  behavior changes.
- PASS — the helper prefers the current frozen `PyQt6/Qt6/plugins` layout and
  retains the older `PyQt6/Qt/plugins` fallback.
- PASS — both Qt plugin roots are bound to the current extracted bundle, so a
  machine-level Qt path cannot redirect platform discovery for the frozen app.
- PASS — the archive contains `qwindows.dll`, the PyQt6 runtime hook, QScintilla,
  the application icon, and the required application modules.
- PASS — no composition, theme, document, plugin-execution, or user-data policy
  was changed.

## Simplification assessment

PASS. One frozen-only helper at the application entry boundary is the smallest
complete change. Adding a second runtime hook, spreading path logic across
composition, or changing the PyInstaller spec would duplicate the existing
runtime-hook responsibility and increase packaging coupling.

## Review limits

The independent Luna/max review window returned `NO_CONCLUSION` after three
bounded waits and was closed. No independent PASS is claimed. Native EXE/Qt
startup and clean-machine behavior remain unrun under the active policy.

## Applicability

Python 3.12/PyQt6/PyInstaller desktop code only. Public embedded-vendor source
applicability is N/A.
