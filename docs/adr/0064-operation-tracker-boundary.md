# ADR-0064: MainWindow operation-tracker boundary

- **Status:** accepted-with-limits; D39 / ARCH-29 / UI-25 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` coordinates several asynchronous paths: document open/save,
workspace navigation, Replace All, session continuity, recovery persistence,
and settings persistence. Those paths intentionally share a monotonic
operation-ID sequence, while only the document/workspace-facing lifecycle
uses one current active operation and stale-completion guard. The coordinator
was still the direct owner of both the counter and the active-ID invariant,
which made the next decomposition step unnecessarily coupled to a 2,300-line
Qt class.

## Decision

Introduce the framework-neutral `presentation.operation_tracker.OperationTracker`
with four narrow operations:

- `reserve()` allocates a fresh monotonic ID for independent worker lifecycles;
- `begin()` allocates and marks one ID as the current active UI operation;
- `complete(id)` clears only a matching current operation;
- `cancel(id)` applies the same stale-ID guard for cancellation.

`MainWindow` delegates only this invariant. It continues to own `_busy`,
`TaskRunner` submission and pending-work semantics, status phases and
notifications, workspace/session generation, service policy, and all result
projection. Existing `_next_operation_id()` call sites remain as a compatibility
facade for independent operations, while `_begin_operation()` and
`_complete_operation()` retain the existing busy/status behavior.

## Invariants

1. Operation IDs remain one process-local monotonic sequence across all
   retained worker submissions.
2. A stale completion or cancellation cannot clear a newer active operation.
3. Workspace cancellation still invalidates its generation and releases the
   existing busy/status barrier; cooperative worker completion remains isolated
   by `TaskRunner` and the existing callback guards.
4. `OperationTracker` imports no Qt, services, widgets, or application policy.
5. No operation result, error mapping, close guard, locale route, plugin
   boundary, or user-visible text moves in this slice.

## Alternatives considered

- **Keep the counter and active ID in MainWindow:** rejected because the
  lifecycle invariant remains embedded in the largest coordinator and every
  future coordinator would need to understand its private fields.
- **Move `_busy`, status phase, or TaskRunner into the tracker:** rejected
  because those are presentation policy and retained-worker lifecycle, not ID
  allocation; moving them would create a second state owner.
- **Create one tracker per operation domain:** rejected because it would
  duplicate stale-guard semantics and change the existing monotonic ID
  sequence; this slice keeps one narrow tracker per MainWindow instance.
- **Add signals/events to the tracker:** rejected because no second consumer
  exists and a pure synchronous value boundary is sufficient.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code, not embedded C/C++ or
firmware. MCU/vendor requirements are not applicable. Public CloudWeGo pages
remain transferable engineering references only; they do not establish a
private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Source/AST probes prove the tracker owns reserve/begin/complete/cancel and
  MainWindow no longer owns the old counter or active-ID fields directly.
- Static inspection proves all previous operation-ID call families still route
  through the same sequence and that TaskRunner, busy, status, and policy
  remain in MainWindow.
- Compile, Ruff, format, handoff, package provenance, and release no-go
  evidence are recorded.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Runtime interleaving, callback timing, UI rendering, screen-reader output,
font metrics, DPI, clean-machine behavior, cross-machine behavior, and
release-owner gates remain unrun. This is a bounded coordinator seam, not a
complete MainWindow rewrite or a claim of enterprise certification.
