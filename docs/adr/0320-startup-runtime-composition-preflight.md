# ADR-0320: Startup runtime-composition preflight

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D284 / ARCH-254

## Context

The existing no-window startup diagnostic proved Python, Qt, QScintilla,
module imports, frozen dependencies, settings metadata, and the application
icon. It did not construct the `DesktopRuntime`, so a constructor-time failure
inside `MainWindow` or one of its presentation adapters could still make a
windowed candidate appear to do nothing while the diagnostic passed.

## Decision

After the existing composition import probe, `--diagnose-startup` runs
`_startup_runtime_composition()`. The probe creates a temporary
`QApplication`, calls the existing `build_desktop_runtime()` composition root,
records only the runtime/window type names and explicit no-window facts, then
stops the runtime and quits only the temporary application in a fail-safe
`finally` boundary.

The probe does not call `DesktopRuntime.start()`, `window.show()`, or
`QApplication.exec()`. It therefore exercises constructor-time wiring and Qt
resource loading without restoring user state, opening a window, entering an
event loop, or duplicating composition policy in the diagnostic layer. The
existing `probe()` wrapper records any construction exception in the report
and preserves the existing exit-code contract.

## Consequences

The documented no-window report now distinguishes import success from full
desktop-graph construction. A future constructor or Qt resource regression is
actionable through the report instead of remaining a silent native-startup
failure. This is a diagnostic boundary, not a claim that a native Windows
window renders or remains alive on a clean machine.

## Public-source applicability

Python 3.12 `try`/`finally` and `pathlib` behavior are applicable first-party
references: <https://docs.python.org/3/reference/compound_stmts.html#the-try-statement>.
Qt application ownership and event-loop behavior are applicable public Qt
references: <https://doc.qt.io/qt-6/qapplication.html>.
No manufacturer requirement applies. This is not embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware work; no certification
claim is made.

## Verification boundary

The runtime-composition source probe passed with
`runtime=DesktopRuntime`, `window=MainWindow`, `window_shown=false`, and
`event_loop_entered=false`. The presentation contract audit, Ruff, formatting,
compileall, project checks, PyInstaller build, PE header, frozen-archive, and
root/dist identity checks passed. Native EXE/Qt startup, clean-machine
behavior, signing, installer, updater, registry, and release-go evidence
remain unrun or open under the active no-launch policy.
