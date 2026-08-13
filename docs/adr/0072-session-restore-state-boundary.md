# ADR-0072: session-restore state boundary

- **Status:** accepted-with-limits; D47 / ARCH-37 / UI-33 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` had accumulated the value state for ordered startup restoration:
the loaded session snapshot, workspace barrier, next document index, active
path, recovery-deferred paths, and the document/operation pairing used by the
asynchronous open callback. The window must still own Qt widgets, services,
TaskRunner dispatch, notifications, startup and close policy, but those
framework-neutral values can be made explicit and independently inspectable.

## Decision

Introduce `presentation.session_restore_tracker.SessionRestoreTracker` as a
Qt-free, service-free state boundary. It owns the immutable snapshot reference,
ordered document cursor, active-path projection, workspace barrier, deferred
recovery paths, and one pending session document bound to its matching open
operation ID. `MainWindow` remains the coordinator: it owns recovery/session
services, workspace and document operations, generic busy/status state,
notifications, tab projection, startup/close guards, and all result policy.

The tracker exposes a read-only `has_remaining_documents` check so queue
inspection cannot consume a document. `next_document()` is the only ordered
cursor operation; `set_pending_document()`, `bind_open_operation()`, and
`take_open_document()` make the asynchronous callback identity explicit.

## Invariants

1. Recovery decisions precede session restoration, and the workspace barrier
   still gates document opening.
2. Session paths are consumed exactly once and in manifest order; missing,
   deferred, duplicate, invalid, and failed paths continue the existing queue.
3. A session-open callback consumes only the operation ID/document binding that
   matches its operation; stale generic callbacks remain ignored by the
   existing operation boundary.
4. Active-tab selection, caret projection, tab reuse, initial-document
   fallback, session-save request, notifications, and close policy remain in
   `MainWindow`.
5. The tracker imports no Qt, filesystem service, TaskRunner, or notification
   surface.

## Alternatives considered

- **Leave all fields in `MainWindow`:** rejected because the coordinator would
  continue to mix framework policy with a reusable ordered-state invariant.
- **Move startup orchestration into a new service:** rejected because it would
  move Qt callbacks, service calls, notifications, and policy beyond this
  smallest complete boundary.
- **Use a generic dictionary/state object:** rejected because typed methods
  make queue consumption and callback binding contracts reviewable.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable implementation reference is the project's existing local
session contract in `docs/adr/0023-local-session-continuity.md` and the
application acceptance contract in `docs/agent-team/acceptance.json`.

## Verification target

- A source probe proves the tracker is Qt-free, typed, and limited to restore
  values while MainWindow retains orchestration dependencies and policy.
- A queue-order/callback-binding probe proves inspection does not consume the
  first document and a pending document is bound before async dispatch.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence are
  recorded.
- Parent and independent reviews record conclusions or explicit no-conclusion
  states.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active no-launch policy.

## Limits and simplification

This is the smallest complete extraction of the duplicated restore value state.
It intentionally leaves `_session_restore_tabs`, tab selection, service calls,
notifications, and startup/close lifecycle in MainWindow because those values
are widget- or policy-owned. No new persistence schema, signal, thread,
animation, or coordinator was introduced. Native callback timing, actual Qt
startup, recovery/file I/O, cross-machine behavior, and release-owner gates
remain unrun.
