# ADR-0330 — Reuse one QApplication across startup diagnostics

## Context

D294 reproduced a Qt warning in the source startup diagnostic:
`Qt6111ThemeChangeObserverWindow` could not be registered because the
diagnostic created one `QApplication` for runtime composition, released it,
and then created a second instance for startup restore. Each individual probe
passed, but the diagnostic itself did not model a clean Qt application
lifecycle.

## Decision

`_run_startup_diagnostic()` now acquires one diagnostic-owned
`QApplication` before running the probe sequence. The existing runtime
composition and startup-restore helpers reuse `QApplication.instance()` when
it is present. The diagnostic closes only an application it created, in a
`finally` block; an application supplied by an outer caller is never quit.

The normal `app.main()` path remains unchanged: it creates exactly one
application for the real desktop runtime and owns its event loop and runtime
cleanup. The diagnostic remains no-window and never calls `application.exec()`.

## Alternatives rejected

- Setting `QT_QPA_PLATFORM=offscreen` inside the production diagnostic would
  hide the lifecycle defect and change the platform contract being inspected.
- Keeping one application per helper preserves duplicate registration risk and
  makes lifecycle ownership implicit.
- Adding a global application singleton would leak diagnostic state across
  callers and complicate process reuse.
- Changing TaskRunner exception handling in this slice would mix an unrelated
  worker policy decision into Qt diagnostic lifecycle work.

## Public-source applicability and limits

This is Python 3.12/PyQt6 desktop code. No embedded C/C++, MCU, BSP/HAL,
RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE, certification, or
private ByteDance-standard claim applies. Python `try/finally` and Qt
`QApplication` ownership are engineering references only. The change does not
prove native EXE startup, clean-machine behavior, or interactive window
rendering.

## Consequences

The source startup diagnostic now has one explicit Qt owner and no longer emits
the reproduced duplicate-window-class warning. Independent native startup and
the separate TaskRunner `BaseException` policy remain open review surfaces.
