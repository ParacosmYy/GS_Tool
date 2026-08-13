# ADR-0068: workspace-search lifecycle boundary

- **Status:** accepted-with-limits; D43 / ARCH-33 bounded coordinator slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` still carried three independent workspace-search lifecycle fields:
the active operation ID, a root-generation counter, and a cooperative
`threading.Event`. The surrounding policy was correctly owned by MainWindow,
but the lifecycle invariant was spread across start, cancel, invalidate,
success, failure, and close paths. That made the stale-callback contract
harder to audit and duplicated the pattern already isolated for other
operation domains.

## Decision

Add the Qt-free `WorkspaceSearchOperationTracker` in the presentation layer.
It owns only four facts and transitions:

- one active operation ID and whether a callback is in flight;
- a monotonically increasing generation returned by `begin()`;
- one cooperative cancellation event;
- `finish()` classification as `stale`, `invalidated`, or `current`.

`cancel()` sets the event but keeps the generation current so a normal user
cancellation result can still be projected. `invalidate()` increments the
generation before requesting cancellation so a root change or other
invalidation projects the eventual callback as cancelled. `finish()` clears
state only when the operation ID is still current; an old callback cannot
clear a newer operation.

MainWindow remains the owner of `WorkspaceSearchService`, query validation,
`TaskRunner`, `WorkspaceSearchSurface`, root containment, notifications,
startup/close guards, and all result policy. The tracker does not import Qt,
filesystem, application services, or UI surfaces.

## Invariants

1. A search cannot begin while another search callback is in flight.
2. User cancellation does not itself invalidate the generation.
3. Root invalidation changes the generation before cancellation is requested.
4. A stale operation ID returns `stale` and leaves the current operation
   untouched.
5. A current ID with an old generation returns `invalidated` and clears only
   that current operation.
6. A current ID and generation returns `current` and clears the lifecycle
   state exactly once.
7. No tracker transition owns notification, surface, containment, or worker
   submission policy.

## Alternatives considered

- **Keep the fields in MainWindow:** rejected because callback lifecycle
  correctness remains distributed across five paths.
- **Move search policy into WorkspaceSearchSurface:** rejected because it
  would mix Qt projection with service, worker, and root policy.
- **Reuse generic OperationTracker only:** rejected because search requires a
  generation/invalidation distinction and a cooperative cancellation event.
- **Introduce a broad application coordinator:** rejected because this slice
  needs only a small framework-neutral lifecycle state machine.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source probe proves the tracker is Qt/service/surface-free, has all
  lifecycle transitions, and is the only owner of the three former search
  fields.
- A boundary probe proves MainWindow retains TaskRunner, service, surface,
  containment, notification, startup, and close policy.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence
  are recorded.
- Independent review records the concurrency result or an explicit
  no-conclusion outcome.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits and simplification

The smallest safe change is one focused tracker plus direct delegation at the
existing callback boundaries. It removes duplicated lifecycle bookkeeping
without creating an event bus, service locator, or broad coordinator. Native
thread timing, queued Qt delivery, close-event interleavings, cross-machine
behavior, and runtime search evidence remain unrun under the no-launch policy.
