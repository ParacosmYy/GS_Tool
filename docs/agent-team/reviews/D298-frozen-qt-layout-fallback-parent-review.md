# D298 parent review — frozen Qt layout fallback

## Scope

Reviewed the installed PyInstaller PyQt6 runtime-hook layout order, QuillForge
plugin discovery, `_startup_qt_runtime_dependencies()`, D297 preflight,
diagnostic report shape, static layout contract, and the rebuilt archive.

## Findings

- PASS — the dependency inventory now checks `Qt6/bin` before `Qt/bin`, matching
  the public PyInstaller hook and existing plugin-path helper.
- PASS — a complete candidate returns the same success fields and current Qt6
  required paths as before.
- PASS — QScintilla remains required in both candidate evaluations.
- PASS — if neither layout is complete, the report remains failed and selects
  the candidate with the fewest missing files, preserving actionable output.
- PASS — D297 fail-fast semantics now use the layout-compatible inventory.
- PASS — source startup/file-open diagnostics and all presentation contracts
  remain green; the current archive contains the Qt6 candidate paths.
- PASS — no normal composition, theme, document, plugin, or user-data policy
  changed.

## Simplification assessment

PASS. A small candidate loop inside the existing dependency helper is the
smallest complete fix. Duplicating layout logic in the preflight or adding a
new packaging abstraction would increase drift and coupling.

## Review limits

The independent Luna/max review window returned `NO_CONCLUSION` after three
bounded waits and was closed. No independent PASS is claimed. A physical
legacy-layout bundle, native EXE startup, and clean-machine behavior remain
unverified.

## Applicability

Python 3.12/PyQt6/PyInstaller desktop code only. Public embedded-vendor source
applicability is N/A.
