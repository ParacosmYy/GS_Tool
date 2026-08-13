# D44 parent review — workspace-navigation lifecycle boundary

- **Delivery:** D44 / ARCH-34
- **Date:** 2026-08-11
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review returned no conclusion

## Scope and architecture decision

D44 extracts only workspace-navigation identity and generation state from
MainWindow into the Qt-free `WorkspaceOperationTracker`. The generic
`OperationTracker` remains the owner of the shared active operation and busy/
status lifecycle through MainWindow. WorkspaceService, TaskRunner,
WorkspaceSurface, session-restore barrier, notifications, containment, and
result policy remain in MainWindow.

The required Terra architecture consultation was attempted with Pauli the 2nd
/ Terra max for a concurrency and callback-order review. Two bounded waits
returned no conclusion and the agent was closed; no architecture PASS is
claimed. The parent decision is limited to the state-machine contract in
ADR-0069.
The independent review window (Pascal the 2nd / Luna max) also returned no
conclusion after two bounded waits; no child PASS or FAIL is claimed. The
explicit record is `D44-workspace-navigation-lifecycle-independent-review.md`.

## Static review findings

- `begin()` validates the positive operation ID, rejects a second active
  navigation, increments generation, and records the ID.
- `invalidate()` increments generation and consumes the active workspace ID;
  MainWindow then calls generic `OperationTracker.cancel()` to preserve the
  existing busy/status release path.
- `finish()` returns `stale` without clearing a different operation and
  distinguishes invalidated versus current generation for matching callbacks.
- Open, directory, and failure callbacks retain the existing order: generic
  operation completion guard, workspace lifecycle classification, loading
  projection, generation branch, result validation, notification/session
  policy.
- The tracker has no Qt, service, filesystem, TaskRunner, surface, or
  notification dependency; MainWindow retains all of those boundaries.

## Simplification assessment

One focused tracker is the smallest complete change. Reusing the D43 search
tracker would incorrectly imply cooperative event cancellation; moving generic
busy/status or session policy would create a second state owner. No further
safe simplification is required for D44.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material remains engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
Applicability is recorded in ADR-0069.

## Authorized non-destructive validation

- D44 workspace-lifecycle source/boundary probe — PASS.
- Compile, Ruff, and format checks — PASS for the changed source slice.
- No unit tests, mocks, fixtures, test-only assets, QApplication, screenshots,
  deployment, or hardware operation were created or run.

## Limits

Independent callback-order review, native queued delivery, real thread timing,
close-event interleavings, runtime navigation, cross-machine appearance, and
release-owner gates remain unrun under the active no-launch policy.
