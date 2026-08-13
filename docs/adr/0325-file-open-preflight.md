# ADR-0325: Explicit file-open preflight

- Status: accepted-with-limits
- Date: 2026-08-12
- Decision owners: Architect, product, developer, QA

## Context

The project already connected workspace file activation and explicit desktop
launch paths to the asynchronous document-open boundary, but evidence was
mostly static. The reported user symptom—folders opening while files did not—
needed a reproducible source diagnostic that exercised a real regular file.

## Decision

Add `--diagnose-file-open <path> --report <path>` to the existing application
diagnostic dispatcher. The probe validates the requested path as a regular
file, builds the normal desktop runtime, and delegates to
`DesktopRuntime.preflight_startup_paths()`. That seam preserves the normal
restore → command refresh → queued startup-path order and reuses
`MainWindow.open_startup_paths()` plus the shared bounded Qt completion wait.

The report contains only the requested path and operational metadata: file
presence, recovery-candidate count, restore/open completion, pending work,
queue drain, and tab counts. It does not write session/settings data, show a
window, or enter `QApplication.exec()`. If recovery candidates require a modal
user choice, the check is explicitly skipped rather than opening the prompt.

## Consequences

The source diagnostic can now prove that a regular file reaches a tab through
the same path used by normal launch. Directory input and missing paths fail
with an actionable diagnostic instead of being silently treated as files.
Normal workspace/file activation and the runtime `start()` path are unchanged.

## Public-source applicability

Python 3.12 `pathlib`/argument behavior and public Qt 6 lifecycle references are
applicable. No manufacturer requirement applies: this is Python/Qt desktop
code, not embedded C/C++, MCU/BSP/HAL/RTOS, or firmware.
