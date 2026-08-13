# ADR-0126: Recovery-write result projection coordinator boundary

- **Status:** accepted-with-limits; D101 / ARCH-75 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D90 and D93 moved discarded-snapshot classification, capture abort, document
and write lifecycle release, and pending-delete forwarding into the Qt-free
`RecoveryWriteCoordinator`. The valid writer callbacks still left one mixed
policy block in `MainWindow`: owner liveness, dirty state, snapshot identity,
content-version comparison, deletion, and feedback were interleaved with the
writer boundary.

## Decision

Extract the post-write saved/failed policy into the Qt-free
`RecoveryProjectionCoordinator[OwnerT]`. It receives narrow callbacks and
preserves the existing policy order:

1. A discarded saved result schedules deletion and stops.
2. A saved result for a no-longer-live owner schedules deletion and stops.
3. For a live owner, dirty state is read before snapshot identity, preserving
   the former evaluation order.
4. A stale snapshot identity schedules deletion.
5. A matching clean owner clears its recovery snapshot.
6. A matching dirty owner emits the existing informational feedback only when
   its content version is newer than the written version.
7. A discarded failure is silent; a non-discarded failure notifies only when
   its owner is still live.

`RecoveryWriteCoordinator` remains responsible for discarded/capture/document/
write lifecycle and pending-delete release. `MainWindow` remains the
composition root and supplies concrete tab-surface, tab-state, editor,
recovery-delete, snapshot-clear, notification, and close-policy callbacks.

## Invariants

1. `recovery_projection_coordinator.py` imports no PyQt6, widget, editor,
   `RecoveryService`, persistence store, or `TaskRunner` implementation.
2. The projection coordinator receives only a result already classified by
   D90's writer lifecycle; it does not consume tracker state, retry, repair,
   swallow callback exceptions, or change snapshot persistence semantics.
3. Delete scheduling remains idempotent/admitted by the existing
   `RecoveryCaptureTracker`/`RecoveryDeleteCoordinator` path; the new module
   only selects the existing deletion callback.
4. Dead owners cannot receive clear, newer-edit, or failure feedback; stale
   snapshot IDs cannot clear a newer live snapshot.
5. No recovery schema, capture cadence, editor state, close guard, notification
   contract, theme token, locale contract, or service API changes.

## Alternatives considered

- **Keep both policy methods in `MainWindow`:** rejected; D90's lifecycle
  boundary would continue to expose a mixed valid-result policy block.
- **Move policy into `RecoveryWriteCoordinator`:** rejected; tracker/write
  lifecycle and owner/tab presentation policy have different reasons to
  change and the writer coordinator must remain generic.
- **Introduce a recovery domain policy/service:** rejected; this slice only
  names an existing presentation ordering and adds no new state or use case.
- **Pass a generic event bus or context object:** rejected; explicit callbacks
  keep ownership and side effects visible at the composition root.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance gate is recorded as N/A for this change.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Halley the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. Mechanical import/format
  confirmations were separately assigned to Plato the 3rd and Maxwell the 3rd;
  both also returned `NO_CONCLUSION`. No child architecture PASS is claimed.
- Independent review: Avicenna the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No independent PASS is
  claimed.
- Parent source review: PASS for D90 lifecycle retention, exact saved/failed
  branch order, live-owner gating, stale snapshot deletion, dirty/version
  handling, Qt-free imports, and direct MainWindow wiring.
- Simplification assessment: PASS. The module names one focused post-write
  projection contract with explicit callbacks; no shared state, context bag,
  generic recovery framework, or duplicate policy was added. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D101-RECOVERY-PROJECTION-SOURCE-PROBE=PASS` covers the Qt-free source
  boundary, direct wiring, and removal of the former MainWindow policy methods.
- `D101-RECOVERY-PROJECTION-ORDER-PROBE=PASS` covers discarded/dead/stale,
  clean/newer-edit, and discarded/live/dead failure ordering.
- Compileall, Ruff, format, package identity, handoff, repository, no-process,
  traceability, and expected release NO-GO evidence are recorded in the D101
  handoff.
- Native recovery capture/write timing, Qt startup, visual feedback, editor
  lifecycle, accessibility, DPI, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner evidence remain unrun
  or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
