# ADR-0346: Explicit safe startup recovery path

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D310 / ARCH-280

## Context

The current source startup and file-open diagnostics pass, and the historical
startup crash caused by the missing `CommandSurface._locale` accessor is
already guarded. Native launch of the current unsigned portable candidate is
still unavailable under the active no-launch policy, so a user who cannot
reach the shell needs a bounded recovery entry point that does not mutate
settings or session state.

## Decision

Recognize an explicit `--safe-mode` launch flag in the application dispatcher
and remove it before Qt argument parsing. The composition root receives the
boolean through the existing `DesktopRuntime` boundary. Safe mode:

- uses immutable `DEFAULT_SETTINGS` instead of the persisted appearance;
- skips built-in plugin activation;
- skips session and recovery restoration;
- creates the initial blank document and keeps explicit startup file paths;
- retains the existing theme, window, event-loop, and cleanup boundaries; and
- never writes settings or session data as part of the mode selection.

Normal startup is unchanged when the flag is absent. Safe mode is a recovery
mechanism, not a second application composition or a permanent settings mode.

## Review and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits and was closed. Parent review and behavior-preserving
simplification assessment are recorded separately. The independent review is
recorded separately and does not claim native launch evidence.

This is Python 3.12/PyQt6 desktop application code. PyInstaller, Qt, and
Python command-line/runtime behavior are engineering references; no embedded
vendor requirement applies. No embedded C/C++, MCU, BSP/HAL, RTOS, MISRA, ISO
26262, ASPICE, certification, or private corporate-standard claim is made.

## Verification and limits

- `D310-SAFE-MODE-PROBE=PASS flag=1 strip=1 default_settings=1 skip_restore=1 skip_plugins=1 startup_paths_preserved=1`.
- `D310-SAFE-MODE-COMPOSITION=PASS safe_mode=true initial_document=true`.
- Source startup and regular-file-open diagnostics pass with no window shown
  and no event loop entered.
- Compileall, Ruff, format, presentation audit, package identity, PE header,
  and frozen archive checks pass.
- Native EXE launch, actual Windows window rendering, safe-mode visual state,
  clean-machine behavior, DLL loading, signing, installer, updater, and
  release-owner gates remain unrun under `software_start_allowed=false`.
- No unit tests, mocks, fixtures, or test harnesses were created or run.

