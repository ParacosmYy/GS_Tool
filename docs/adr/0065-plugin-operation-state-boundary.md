# ADR-0065: plugin operation-state boundary

- **Status:** accepted-with-limits; D40 / ARCH-30 / UI-26 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

MainWindow currently stores three unrelated plugin-operation counters and
in-flight flags directly: catalog scanning, catalog approval/revocation, and
isolated host probing. Each path needs its own stale-callback guard because
those operations may have independent lifetimes. The duplicated fields are
presentation lifecycle state, but they sit beside plugin services, global
notifications, and UI projection in the largest coordinator.

## Decision

Introduce the Qt-free `PluginOperationTracker` in
`presentation/plugin_operation_tracker.py`. It exposes one closed set of
operation kinds and three operations:

- `begin(kind)` increments that kind's monotonic sequence and marks it active;
- `in_flight(kind)` reports only whether that kind currently owns a callback
  boundary;
- `complete(kind, operation_id)` clears only a matching active ID and rejects
  stale delivery.

The tracker owns no plugin service, trust/approval/enablement policy,
TaskRunner, notification, command refresh, dialog, or Qt object. MainWindow
continues to own all those policies and callbacks. The three plugin operation
sequences remain independent; this is intentional because catalog scan,
catalog governance, and host probing can overlap and must not share D39's
single active document/workspace lifecycle.

## Invariants

1. Each operation kind keeps its existing monotonic local ID sequence.
2. A stale callback for one kind cannot clear a newer callback of that kind.
3. Completing one kind never changes another kind's in-flight state.
4. Plugin trust, approval, enablement, external execution, containment,
   command refresh, error text, and notification severity remain owned by
   their existing application/presentation callers.
5. The tracker has no reverse dependency on MainWindow, PluginSurface, Qt, or
   infrastructure.

## Alternatives considered

- **Keep six fields in MainWindow:** rejected because the same lifecycle
  invariant is repeated and cannot be reused by a future plugin coordinator.
- **Reuse D39's single active OperationTracker:** rejected because these three
  plugin paths have independent concurrency domains and may legitimately
  overlap.
- **Move plugin services or policy into the tracker:** rejected because it
  would create a service locator/state owner and weaken the existing trust and
  execution boundaries.
- **Use a free-form string-keyed dictionary:** rejected because a closed typed
  operation-kind contract catches accidental cross-domain completion.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation lifecycle code, not embedded C/C++ or
firmware. MCU/vendor requirements are not applicable. Public CloudWeGo pages
remain transferable engineering references only; they do not establish a
private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source probe proves the closed kinds, per-kind monotonic IDs, independent
  in-flight states, stale completion rejection, and MainWindow delegation.
- Static inspection proves PluginSurface, TaskRunner, notifications, command
  refresh, and plugin policy remain outside the tracker.
- Compile, Ruff, format, handoff, package provenance, and release no-go
  evidence are recorded.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Runtime callback interleaving, native dialog behavior, accessibility, font
metrics, DPI, clean-machine behavior, cross-machine behavior, and release
owner gates remain unrun. This is a small lifecycle-state boundary, not a
complete plugin coordinator or security isolation claim.
