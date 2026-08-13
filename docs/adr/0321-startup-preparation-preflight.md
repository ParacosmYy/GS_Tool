# ADR-0321: Startup preparation preflight

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D285 / ARCH-255

## Context

D284 extended the no-window diagnostic through `DesktopRuntime` and
`MainWindow` construction, but the normal runtime performs two additional
pre-show steps: it binds host capabilities/activates built-in plugins and
refreshes command projections. A failure in either step could still leave a
windowed candidate without a useful diagnostic.

## Decision

`DesktopRuntime` now owns two named pre-show lifecycle stages:

1. `prepare_startup()` binds the active-document and notification capabilities
   and activates the registered built-in plugins.
2. `refresh_startup_commands()` refreshes command projections.

Normal `start()` calls these stages in the existing order around session
restore: prepare, restore startup state, refresh commands, show, then route
explicit startup paths. The no-window runtime probe calls only prepare and
command refresh, then stops the runtime. It does not restore session state,
start recovery/session workers, show a window, or enter the event loop.

This keeps the diagnostic on the same owned lifecycle seams without copying
plugin or command setup into `app.py` and without moving application policy
into presentation code.

## Consequences

Constructor-time and pre-show plugin/command failures are now reported by the
existing `--diagnose-startup` command. Normal startup ordering remains guarded
by a static contract. The probe intentionally does not prove asynchronous
session/recovery behavior, native window lifetime, or clean-machine startup.

## Public-source applicability

Python 3.12 first-party callable/exception behavior and public Qt 6
`QApplication` lifecycle semantics are applicable references:
<https://docs.python.org/3/reference/compound_stmts.html> and
<https://doc.qt.io/qt-6/qapplication.html>.
No manufacturer requirement applies. This is not embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware work; no certification
claim is made.

## Verification boundary

The source no-window diagnostic passed with runtime composition and startup
preparation both successful, while `window_shown=false` and
`event_loop_entered=false`. The presentation audit, Ruff, formatting,
compileall, project checks, PyInstaller build, PE header, frozen archive, and
root/dist identity checks passed. Native EXE/Qt startup, asynchronous restore,
clean-machine behavior, signing, installer, updater, registry, and release-go
evidence remain unrun or open under the active no-launch policy.
