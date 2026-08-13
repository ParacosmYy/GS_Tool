# ADR-0328 — Stop periodic UI activity before runtime deactivation

## Context

D291 moved normal `runtime.start()` into the entrypoint cleanup boundary. That
made `DesktopRuntime.stop()` reachable when startup fails after the runtime has
been constructed, but `stop()` previously deactivated plugins only. The
`MainWindow` owns a periodic recovery timer and a debounced session-save timer;
the ordinary close guard stops them on an accepted close, while a partial
startup failure has no close event to perform that step.

`TaskRunner` also owns queued worker completions that target the Qt UI thread.
There is no safe generic force-stop operation in the current application
contract, and waiting or killing those workers in composition would change
callback and shutdown semantics.

## Decision

Expose one presentation lifecycle port,
`MainWindow.stop_background_activity()`, that stops only the existing periodic
recovery and session-save timers. `DesktopRuntime.stop()` calls this port before
`plugins.deactivate_all()` on every normal or exceptional exit.

The boundary is intentionally idempotent and does not wait for or terminate
`TaskRunner` work. Queued callbacks remain governed by the existing Qt object
ownership and event-loop lifecycle. The static presentation audit checks that
the composition shutdown order is explicit and that the port delegates to the
existing timer helper.

## Alternatives rejected

- Calling `MainWindow.close()` would re-enter user close policy and could show a
  modal decision during an exception path.
- Adding a forced `QThreadPool` wait or termination would change worker
  callback semantics and require a new cancellation/ownership contract.
- Duplicating timer stops in `DesktopRuntime` would leak presentation details
  into the composition root.

## Public-source applicability and limits

This is Python/PyQt desktop code, not embedded C/C++ or firmware. No MCU,
vendor SDK, RTOS, BSP/HAL, manufacturer requirement, MISRA, ISO 26262,
ASPICE, or certification claim applies. Python `try`/`finally` behavior and
Qt QObject/timer ownership are engineering references only:
<https://docs.python.org/3/reference/compound_stmts.html#the-try-statement>
and <https://doc.qt.io/qt-6/qtimer.html>.

Native EXE startup, clean-machine behavior, and worker teardown under a real
window close remain outside the authorized non-destructive verification scope.
