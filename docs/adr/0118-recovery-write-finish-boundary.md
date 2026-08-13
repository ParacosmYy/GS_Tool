# ADR-0118: Recovery-write finish boundary

- **Status:** accepted-with-limits; D93 / ARCH-68 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The D90 `RecoveryWriteCoordinator` classified writer callbacks and preserved
their ordering, but the injected MainWindow `_finish_recovery_write` callback
still performed `RecoveryCaptureTracker.finish_write()` and drained a pending
delete. That left write-release lifecycle in the Qt shell even though the
coordinator already owned the surrounding success/failure transition.

## Decision

Complete the existing Qt-free `RecoveryWriteCoordinator[JobT, OwnerT]` boundary.
It now calls `RecoveryCaptureTracker.finish_write()` exactly once and forwards
any returned pending-delete data through an explicit generic callback
`(snapshot_id, owner | None, success_message | None)`. MainWindow retains only
the deletion request admission, RecoveryService dispatch, and delete callback
policy.

## Invariants

1. `RecoveryWriteCoordinator` remains free of PyQt6, editor widgets,
   RecoveryService, and concrete recovery jobs.
2. Success ordering remains discarded consumption → document release → write
   release/pending-delete forwarding → saved policy projection.
3. Failure ordering remains discarded consumption → matching capture abort →
   document release → write release/pending-delete forwarding → failed policy
   projection.
4. Pending-delete forwarding preserves the original snapshot id, optional owner,
   and success message; request admission and actual delete dispatch remain in
   MainWindow/RecoveryDeleteCoordinator.
5. D91 producer-side capture abort remains separate from D93 writer finish;
   neither coordinator owns the other's failure semantics.

## Alternatives considered

- **Keep `_finish_recovery_write` in MainWindow:** rejected; tracker write
  lifecycle remained split from the coordinator's callback ordering.
- **Move delete request admission or RecoveryService into the coordinator:**
  rejected; persistence and deletion policy are application/service concerns.
- **Create another finish-only coordinator:** rejected; the existing
  `RecoveryWriteCoordinator` is the natural owner and avoids another layer.
- **Change discarded/capture/document ordering:** rejected; this slice only
  closes the already documented D90 write-finish seam.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Cicero the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Zeno the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for single tracker release, pending-delete data
  preservation, D90 success/failure order, and separation from D91 capture
  abort.
- Simplification assessment: the existing D90 coordinator now owns its complete
  write lifecycle without adding a new class or framework. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D93-RECOVERY-WRITE-FINISH-QT-FREE-BOUNDARY-PROBE=PASS` covers tracker
  release ownership, pending-delete seam, MainWindow removal, and the existing
  Qt-free coordinator boundary.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D93 handoff.
- Native writer/capture interleaving, channel backpressure, delete timing,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
