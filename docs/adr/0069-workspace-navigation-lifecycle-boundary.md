# ADR-0069: workspace-navigation lifecycle boundary

- **Status:** accepted-with-limits; D44 / ARCH-34 bounded coordinator slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

After the D43 search extraction, MainWindow still stored workspace-navigation
operation identity and generation directly in `_workspace_operation_id` and
`_workspace_generation`. The generic `OperationTracker` correctly owns the
shared busy/status operation invariant, but workspace navigation also needs a
domain-specific generation so stale open/list callbacks cannot project an old
directory.

## Decision

Add the Qt-free `WorkspaceOperationTracker` in the presentation layer. It owns
only the active workspace operation ID, generation allocation, invalidation,
and `stale`/`invalidated`/`current` callback classification.

MainWindow continues to own the generic `OperationTracker` and `_busy`,
TaskRunner submission, WorkspaceService, WorkspaceSurface, workspace
containment, notifications, startup guards, close policy, session-restore
barrier, and result projection. On user cancellation MainWindow invalidates
the workspace tracker, then uses the generic tracker to release the existing
busy/status barrier. The worker callback remains isolated by TaskRunner and
the generic operation-ID guard.

## Invariants

1. `begin()` increments generation and records one active workspace operation.
2. `invalidate()` increments generation and consumes the active workspace ID;
   the caller remains responsible for generic busy/status cancellation.
3. `finish()` returns `stale` without clearing a different current operation.
4. Matching ID plus an old generation returns `invalidated`; matching ID and
   current generation returns `current`.
5. Workspace open, directory, and failure callbacks clear UI loading and
   session barriers exactly where the existing MainWindow policy did.
6. No tracker transition owns a service call, worker submission, widget
   update, containment decision, notification, or session policy.

## Alternatives considered

- **Keep workspace fields in MainWindow:** rejected because navigation stale
  state remains distributed in the largest Qt coordinator.
- **Move generic busy/status policy into the workspace tracker:** rejected
  because shared operation policy belongs to `OperationTracker`/MainWindow.
- **Reuse the D43 search tracker:** rejected because workspace navigation has
  no cooperative event and its cancellation must bridge to the generic active
  operation guard.
- **Move navigation policy into WorkspaceSurface:** rejected because it would
  mix service, worker, containment, and session-restore policy into a widget
  composition boundary.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source lifecycle/boundary probe proves the tracker transitions, no old
  MainWindow fields, generic cancellation bridge, and retained policy owners.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence
  are recorded.
- Independent review records callback-order findings or explicit
  no-conclusion status.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits and simplification

The smallest safe change is one focused tracker plus direct delegation at the
existing navigation callbacks. It removes duplicated generation/identity
bookkeeping without introducing a broad coordinator or changing the generic
busy/status tracker. Native queued delivery, thread timing, close-event races,
runtime navigation, cross-machine behavior, and release-owner gates remain
unrun under the no-launch policy.
