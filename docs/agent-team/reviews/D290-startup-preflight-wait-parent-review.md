# D290 parent review — startup preflight wait refinement

## Scope

Reviewed `MainWindow._wait_for_startup_preflight`, the Qt event-processing
flags, the wake-timer cleanup, and the updated static presentation contract.

## Findings

- PASS — the change addresses a measured busy-spin rather than changing
  document-open or restore behavior.
- PASS — `AllEvents | WaitForMoreEvents` keeps worker completions and queued Qt
  callbacks deliverable while the unparented 2ms timer guarantees a periodic
  deadline check when the queue is otherwise empty.
- PASS — the monotonic timeout, pending-work predicates, report fields, and
  `finally` timer cleanup remain intact; the report explicitly labels the
  bound as `timeout_mode=soft`.
- PASS — no `show()` or `QApplication.exec()` was added; normal
  `DesktopRuntime.start()` does not use the helper.
- PASS — the static audit now inspects the specific wait method with AST
  control-flow checks for the wake flag, interval/start/stop, monotonic
  deadline, timeout exception, `finally`, and report keys.

## Review limits

The independent Luna/max reviewer returned `NO_CONCLUSION` after two bounded
wait windows and was closed. Native EXE/Qt launch remains intentionally
unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
