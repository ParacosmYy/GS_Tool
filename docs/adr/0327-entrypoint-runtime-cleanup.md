# ADR-0327 — Keep normal runtime startup inside the cleanup boundary

## Context

`quillforge.app.main()` constructs the desktop runtime, starts it, and then
enters the Qt event loop. Before D291, `runtime.start()` ran before the
`try/finally` that called `runtime.stop()`. A failure while activating a
plugin, restoring session state, showing the window, or queueing an explicit
startup path could therefore leave the partially started runtime outside its
normal cleanup boundary.

`DesktopRuntime.stop()` already delegates to the idempotent plugin-manager
deactivation boundary. The early `build_desktop_runtime()` call remains
outside the boundary because no runtime object exists if composition itself
fails; the existing entrypoint failure reporter covers that earlier failure
class.

## Decision

Move only `runtime.start()` into the existing `try` block. The normal order is
unchanged:

1. parse the launch request;
2. construct `QApplication` and `DesktopRuntime`;
3. start the runtime;
4. enter `application.exec()`; and
5. always call `runtime.stop()` after either startup or event-loop failure.

The entrypoint AST audit now requires exactly one `runtime.start()` call in a
`try` body together with `application.exec()`, with `runtime.stop()` in that
same `finally` body. This is a development-time lifecycle guard, not a second
runtime implementation.

## Alternatives rejected

- Duplicating a second cleanup block around `runtime.start()` would make the
  lifecycle harder to audit and could introduce double deactivation.
- Catching startup exceptions here would change the existing startup-failure
  reporting owner and could hide the original traceback.
- Moving composition into the `try` would require a nullable runtime and
  broaden the change beyond the concrete cleanup defect.

## Applicability and limits

This is Python/PyQt desktop code, not embedded C/C++ or firmware. No MCU,
vendor SDK, RTOS, BSP/HAL, manufacturer requirement, MISRA, ISO 26262,
ASPICE, or certification claim applies. The public Python language reference
for the `try` statement is an engineering reference for the language control
flow; it is not a vendor requirement:
<https://docs.python.org/3/reference/compound_stmts.html#the-try-statement>.

Native EXE launch, clean-machine startup, shell association, and GUI rendering
remain outside the authorized non-destructive verification boundary.
