# ADR-0329 — Roll back retained tasks when submission fails

## Context

D292 kept periodic UI activity inside the presentation-owned shutdown boundary
and intentionally left `TaskRunner` worker teardown under Qt ownership. A
remaining failure mode was narrower but concrete: `TaskRunner.submit()` added a
task to `_tasks` before connecting the completion signal and starting the
thread-pool runnable. If either Qt operation raised synchronously, the retained
task could leave `pending_count` permanently non-zero even though submission
had failed. A task accepted by the pool could also complete after a start-call
exception, so cleanup must be idempotent.

## Decision

Keep task retention before connection/start so the existing UI-thread-owned
pending contract and race-free completion ordering remain unchanged. Wrap only
the connection/start boundary in `try/except Exception`; on failure, release
the task through one private `_release_task()` boundary and re-raise the
original exception.

Use the same release boundary after the success/failure callback returns. The
helper removes a task and emits `pending_changed` only when the task is still
retained. A late completion after a partially accepted `start()` therefore
cannot reintroduce a stale count or emit a second state transition.

## Alternatives rejected

- Registering the task only after `QThreadPool.start()` would introduce a race
  where a very fast runnable can complete before the UI retention set is
  updated.
- Adding `waitForDone()`, thread termination, or a shutdown barrier would
  change the existing non-blocking close and callback ownership contract.
- Catching `BaseException` would change fatal/interruption semantics and would
  be unrelated to the synchronous submission-retention bug.
- Adding a generic lifecycle abstraction to each coordinator would duplicate
  ownership already centralized in `TaskRunner`.

## Public-source applicability and limits

This is Python 3.12/PyQt6 desktop code. No embedded C/C++, MCU, BSP/HAL,
RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE, certification, or
private ByteDance-standard claim applies. Python exception propagation and Qt
`QThreadPool`/queued-signal behavior are engineering references only; they are
not a compliance basis. Native EXE startup, real thread-pool timing, clean
machine behavior, and forced worker teardown remain outside the authorized
non-destructive verification scope.

## Consequences

Normal callback behavior and callback-before-release ordering remain unchanged.
Synchronous submission failures now leave `TaskRunner` idle and propagate to
existing callers without synthetic success/failure callbacks. The static audit
records the rollback and shared-release contract. A future shutdown/quiescing
slice may still be needed if runtime reuse or direct object destruction becomes
an explicit supported lifecycle, but that is not inferred here.
