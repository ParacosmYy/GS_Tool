# ADR-0113: Recovery-delete coordinator boundary

- **Status:** accepted-with-limits; D88 / ARCH-63 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._schedule_recovery_delete` contained the completion side of
recovery snapshot cleanup: tracker delete completion/failure, owner snapshot
identity clearing, success/error notification, and draining a delete request
queued behind an in-flight write/delete. It also owns request admission,
RecoveryService dispatch, capture/write lifecycle, recovery decisions, and
close policy.

## Decision

Extract delete completion projection into the Qt-free generic
`RecoveryDeleteCoordinator[JobT, OwnerT]`. It receives the existing
`RecoveryCaptureTracker`, owner identity-clearing seam, pending-delete
resubmission seam, and notification sink.

The coordinator owns tracker complete/fail calls, owner snapshot-id clearing,
success/error notification, and pending-request drain. MainWindow retains
delete request admission, operation ID allocation, RecoveryService dispatch,
capture/write state, and all recovery/application policy.

## Invariants

1. `presentation/recovery_delete_coordinator.py` imports no PyQt6 and has no
   RecoveryService, filesystem, dialog, editor, or close capability.
2. A successful delete always releases tracker delete state before clearing an
   owner identity, notifying success, or scheduling a pending request.
3. Owner clearing is identity guarded through an injected seam, so a newer
   snapshot cannot be cleared by an older callback.
4. A pending delete retains its original owner and success-message payload and
   is resubmitted through MainWindow's existing admission policy.
5. A failed delete releases tracker state and preserves the exact existing
   `Recovery cleanup failed: ...` error notification.
6. MainWindow retains `RecoveryCaptureTracker.request_delete`, operation
   dispatch, RecoveryService, capture/write, restore/discard, and close policy.

## Alternatives considered

- **Leave delete callbacks in MainWindow:** rejected; tracker release,
  identity clearing, notice, and pending drain form a focused lifecycle.
- **Move delete request admission or RecoveryService into the coordinator:**
  rejected; those are application/recovery policy and worker dispatch.
- **Move capture/write callbacks into this coordinator:** rejected; capture
  producer backpressure and writer lifecycle have separate identity contracts.
- **Inject `_DocumentTab` directly:** rejected; generic owner/job parameters keep
  the boundary reusable and Qt-free.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Confucius the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Volta the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for tracker release ordering, owner identity
  guard, success/error notification, pending drain, and retention of request
  and RecoveryService policy.
- Simplification assessment: one small coordinator replaces two local callback
  bodies while leaving request admission and worker dispatch visible. No
  further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D88-RECOVERY-DELETE-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free import
  boundary, callback wiring, tracker ownership, old callback removal, and
  retained recovery policy.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D88 handoff.
- Native recovery deletion timing, capture/write interleaving, accessibility,
  DPI, font metrics, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release owner evidence remain
  unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
