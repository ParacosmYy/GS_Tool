# ADR-0331 — Normalize abnormal worker termination at TaskRunner

## Context

`TaskRunner` catches ordinary `Exception` failures, but `_Task.run()` used a
`finally` block that emitted `completed` for every exit. If a worker operation
raised a non-`Exception` `BaseException` such as `SystemExit`, `KeyboardInterrupt`,
or `GeneratorExit`, `task.error` stayed empty and the UI callback was projected
as a false success with a `None` result. The retained task could then violate
the intended failure/status semantics.

All current coordinator dispatch contracts intentionally accept `Exception`,
and the message surface/localization boundary is also typed around
`Exception`. Widening every coordinator and UI contract would be a broad API
change for a worker-boundary defect.

## Decision

Add a private `_TaskTerminationError(RuntimeError)` at the TaskRunner boundary.
`_Task.run()` continues to catch ordinary `Exception` unchanged; a
non-`Exception` `BaseException` is converted to this stable `Exception`
subclass, retaining the original cause and type/detail text, then emitted
through the existing failure callback. The completion signal and existing
callback-before-release order remain unchanged, so `_release_task()` still
releases every retained task exactly once.

This is a worker-boundary normalization, not a global policy to reinterpret
main-thread interrupts. The application does not force-stop threads, change
the Qt event loop, or widen coordinator public callback types.

## Alternatives rejected

- Leaving the `finally` behavior unchanged allows abnormal worker termination to
  appear as success and can leave user-visible state inconsistent.
- Removing the completion signal on `BaseException` risks retaining the task
  forever and blocking the existing close/status guards.
- Widening all coordinator `Failure` aliases and message surfaces to
  `BaseException` would expand the public presentation contract unnecessarily.
- Re-raising the non-`Exception` value from the worker would bypass the existing
  UI failure projection and make pending cleanup dependent on Qt/Python thread
  exception handling.

## Public-source applicability and limits

This is Python 3.12/PyQt6 desktop code. No embedded C/C++, MCU, BSP/HAL,
RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE, certification, or
private ByteDance-standard claim applies. Python exception hierarchy and Qt
`QRunnable` execution are engineering references only. Direct runtime injection
of `BaseException`, native EXE startup, and real thread timing were not run
under the no-test/no-launch policy.

## Consequences

Abnormal worker termination now follows the existing recoverable failure path,
never the success path, and the cross-module callback surface remains stable.
The private cause is retained for diagnostics, while a future policy may decide
whether worker-boundary errors need structured logging or a dedicated localized
message.
