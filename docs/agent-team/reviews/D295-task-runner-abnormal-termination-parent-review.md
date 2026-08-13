# D295 parent review — TaskRunner abnormal termination boundary

## Scope

Reviewed `src/quillforge/presentation/task_runner.py`, the ten existing
`Exception`-typed failure callback families, `MessageSurface`/
`localize_exception`, pending-task release, and the targeted presentation
contract audit.

## Findings

- PASS — non-`Exception` `BaseException` values no longer leave `task.error`
  empty and cannot enter the success callback as `None`.
- PASS — `_TaskTerminationError` is an `Exception` subclass, so existing
  coordinator and message-surface interfaces remain unchanged.
- PASS — the original cause object and exception type/detail are retained on
  the private wrapper for later diagnostics.
- PASS — completion emission and callback-before-release ordering are
  unchanged; `_release_task()` still handles the retained task.
- PASS — no main-thread interrupt policy, Qt event-loop behavior, forced thread
  termination, or coordinator dependency direction was changed.
- PASS — the static audit requires the wrapper, `BaseException` branch, and
  conversion assignment.

## Simplification assessment

PASS. A private boundary wrapper is the smallest complete compatibility fix.
Changing every coordinator type or introducing a second worker state machine
would increase coupling without improving this failure path.

## Architecture consultation and limits

The required Luna/max architecture window returned `NO_CONCLUSION` after three
bounded 60-second waits and was closed. No architecture PASS is claimed. The
parent review therefore relies on direct source/call-site evidence and preserves
the explicit limits around injected abnormal termination and native timing.

## Applicability

Python/PyQt6 desktop code only. No embedded public-vendor requirement or
certification claim applies.
