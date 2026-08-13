# D43 parent review — workspace-search lifecycle boundary

- **Delivery:** D43 / ARCH-33
- **Date:** 2026-08-11
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review returned no conclusion

## Scope and architecture decision

D43 extracts only workspace-search lifecycle facts from MainWindow into the
Qt-free `WorkspaceSearchOperationTracker`. The tracker owns active ID,
generation, cooperative cancellation event, and completion classification.
MainWindow still owns query validation, `WorkspaceSearchService`, `TaskRunner`,
`WorkspaceSearchSurface`, root containment, notifications, startup/close
guards, and result projection.

The required Terra architecture consultation was attempted with Carver the
2nd / Terra max for a concurrency and call-chain review. Two bounded waits
returned no conclusion and the agent was closed; no architecture PASS is
claimed. The parent decision is limited to the explicit state-machine contract
recorded in ADR-0068.
The independent review window (Aristotle the 2nd / Luna max) also returned no
conclusion after two bounded waits; no child PASS or FAIL is claimed. The
explicit record is `D43-workspace-search-lifecycle-independent-review.md`.

## Static review findings

- `begin()` rejects invalid IDs and a second active operation, increments the
  generation, and creates one cancellation event.
- `cancel()` sets the event without changing the generation, preserving the
  normal user-cancel result path.
- `invalidate()` increments generation before cancellation, making root-change
  callbacks classify as `invalidated`.
- `finish()` returns `stale` without touching current state when IDs differ;
  otherwise it clears exactly the matching operation and distinguishes current
  versus invalidated generation.
- MainWindow delegates start/cancel/invalidate/finish and no longer stores the
  former three search lifecycle fields.
- The close guard uses the tracker while MainWindow retains the existing
  close/error policy.
- The tracker imports only dataclasses, `threading.Event`, and typing; no Qt,
  service, filesystem, TaskRunner, surface, or notification dependency enters
  the new boundary.

## Simplification assessment

The focused tracker is the smallest complete change. A generic tracker would
not express generation invalidation plus cooperative cancellation, while a
larger coordinator would move policy unnecessarily. No further safe
simplification is required for D43.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material remains engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
Applicability is recorded in ADR-0068.

## Authorized non-destructive validation

- D43 lifecycle/boundary source probe — PASS after final source integration.
- Compile, Ruff, and format checks — pass for the changed source slice after
  the import-order correction.
- No unit tests, mocks, fixtures, test-only assets, QApplication, screenshots,
  deployment, or hardware operation were created or run.

## Limits

Independent callback-order review, native Qt queued delivery, real thread
timing, close-event interleavings, runtime search behavior, and release-owner
gates remain unrun under the active no-launch policy.
