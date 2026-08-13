# ADR-0029: TaskRunner-backed shell status projection

## Context

The shell status rail originally followed only the exclusive document-operation
boundary. Recovery, session, workspace-search, settings, catalog, and plugin
host work also runs through `TaskRunner`; while those tasks were retained, the
rail could still show `READY`.

## Decision

`TaskRunner` exposes one presentation-owned `pending_changed` signal. It emits
after a task is retained and again after the queued completion callback returns
and the task is released. `MainWindow` uses that signal as the single status
projection input for all retained background work.

The status priority is:

1. `WORKING` while the shell is busy or any `TaskRunner` task remains retained;
2. `ATTENTION` when the active document is dirty and no work is retained;
3. `READY` when the shell is idle and the active document is clean.

Recoverable modal errors may still project `ERROR`. The signal does not replace
the close guard, cancellation token, operation IDs, or worker ownership. The
`TaskRunner` pending set remains UI-thread-owned and continues to include the
queued completion-delivery window.

## Consequences

- Every current and future `TaskRunner` operation receives the same status
  projection without duplicating lifecycle calls at each call site.
- A task cannot make the shell appear idle before its completion callback has
  drained, matching the close-safety contract.
- Runtime visual, accessibility, DPI, and clean-machine evidence remain
  separate release/runtime gates under the current no-launch policy.
