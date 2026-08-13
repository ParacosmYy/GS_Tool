# ADR-0322: Editor-shell startup preflight

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D286 / ARCH-256

## Context

D285 validated desktop construction, built-in plugin preparation, and command
refresh, but the first `EditorWidget`/QScintilla instance is created only when
session/recovery restoration finishes. A failure in the editor adapter, lexer,
font, theme palette, or tab projection could therefore still appear as a
window that never becomes usable.

## Decision

`MainWindow.preflight_editor_shell()` reuses the existing
`ensure_initial_document()` path to construct one empty untitled editor tab.
It does not restore a session, open a filesystem path, show the window, or
enter the event loop. Its `finally` block stops the existing recovery and
session-save timers through `_stop_close_timers()`. No document is saved: the
new document is in-memory and the event loop is never entered.

`DesktopRuntime.preflight_editor_shell()` owns the composition-level lifecycle
seam and delegates the presentation-specific construction to `MainWindow`.
The normal startup path is unchanged; it still lets session/recovery restore
decide whether and when an initial document is created.

## Consequences

The no-window diagnostic now exercises the first QScintilla/editor/theme/font
construction boundary and reports failures before native display. The
diagnostic deliberately creates no user document on disk and cleans its local
timers. It still does not prove asynchronous restore completion, native window
lifetime, rendering, or cross-machine startup.

## Public-source applicability

Python 3.12 first-party `try`/`finally` semantics and public Qt 6 `QTimer`/
`QApplication` lifecycle documentation are applicable references:
<https://docs.python.org/3/reference/compound_stmts.html> and
<https://doc.qt.io/qt-6/qtimer.html>.
No manufacturer requirement applies. This is not embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware work; no certification
claim is made.

## Verification boundary

The source diagnostic passed with `editor_shell_prepared=true`,
`window_shown=false`, and `event_loop_entered=false`. The presentation audit,
Ruff, formatting, compileall, project checks, PyInstaller build, PE header,
frozen archive, and root/dist identity checks passed. Native EXE/Qt startup,
asynchronous restore, clean-machine behavior, signing, installer, updater,
registry, and release-go evidence remain unrun or open under the active
no-launch policy.
