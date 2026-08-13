# ADR-0022: TaskRunner lifecycle guard

- **Status:** accepted with limits for D7.4.2; runtime and broader environment evidence remain open
- **Date:** 2026-08-09
- **Owners:** Architect, Developer 1, Developer 2, QA

## User outcome

When the user cancels a workspace operation or requests application shutdown,
QuillForge must not destroy the main window while a queued worker or its Qt
completion callbacks are still live. The UI remains responsive, reports that a
background operation is still finishing, and accepts a later close request
after the worker boundary has fully drained.

## Decision

`TaskRunner` is the single lifecycle observation boundary for submitted
`QRunnable` work. It exposes a read-only `pending_count` and
`has_pending_work()` view backed by the same `_tasks` set that retains tasks
until one queued completion slot has delivered the callback and released the
task. Success/failure dispatch and cleanup happen in that one slot, so cleanup
does not depend on the relative ordering of multiple posted signals. Task
ownership, callback marshalling, and `setAutoDelete(False)` remain explicit.

`MainWindow.closeEvent()` keeps its specific cancellation and dirty-document
guards for user feedback and stale-result handling, then consults
`TaskRunner.has_pending_work()` as the final submitted-worker guard. A close
request is rejected with a bounded message while any task remains pending. It
never calls `QThreadPool.waitForDone()` on the UI thread, force-terminates a
worker, or treats a cooperative cancellation request as completion. Operation
IDs and generations continue to discard stale callbacks while the window
remains alive.

When the cancelled workspace operation belongs to the startup session-restore
barrier, `MainWindow._cancel_workspace_operation()` releases that barrier after
invalidating the operation-specific state. The stale workspace completion is
still retained by `TaskRunner` and discarded by its operation/generation guard;
session restoration may continue without waiting on a callback that can no
longer own the active operation.

This removes lifecycle knowledge duplication from the presentation shell while
preserving existing operation-specific state for user feedback and stale-result
handling.

## Scope and limits

In scope: a read-only TaskRunner pending-work contract; a final MainWindow
close guard; workspace-cancellation/queued-completion coverage; startup
session-restore barrier release; bounded status feedback and documentation.

Out of scope: forced worker termination, synchronous shutdown waits, changing
workspace provider cancellation, settings-load asynchronous refactoring,
session persistence, plugin execution, or a claim of crash-proof shutdown
against an operating-system termination. The normal guarantee assumes shutdown
enters `MainWindow.closeEvent()`; direct external destruction,
`QApplication.quit()` paths that bypass that event, and process termination
are not claimed by this increment.

## Verification

- A Qt offscreen smoke will submit a real delayed `QRunnable`, observe pending
  work, release it, process queued completion, and observe the transition to
  idle.
- A MainWindow offscreen smoke will cancel a workspace operation, verify that a
  close event is ignored while the TaskRunner task remains pending, then verify
  that the close guard can accept after queued cleanup.
- A post-fix independent Luna source audit returned PASS for task retention,
  startup-barrier release, stale-completion containment, and non-blocking close.
- `scripts/check.ps1`, package/root hash synchronization, startup, repeat,
  packaged capture, clean-machine preflight, and release handoff checks remain
  required.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are added or
  run by default.
