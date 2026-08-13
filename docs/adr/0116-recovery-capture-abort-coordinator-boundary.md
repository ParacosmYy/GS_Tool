# ADR-0116: Recovery-capture abort coordinator boundary

- **Status:** accepted-with-limits; D91 / ARCH-66 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

Recovery capture can become stale, be cancelled, fail while producing editor
chunks, or fail before its worker starts. The MainWindow callback previously
combined tracker identity protection, snapshot discard/release classification,
channel/session termination, document lifecycle release, and user-facing
failure notification.

## Decision

Extract the capture-abort lifecycle into the Qt-free generic
`RecoveryCaptureAbortCoordinator[JobT, OwnerT]`. It receives generic owner/job
adapters and explicit seams for the tracker, channel, session cancellation,
owner liveness, and failure notification.

The coordinator owns only matching-capture cleanup and its ordering. MainWindow
retains Qt timer scheduling, capture stepping, stale/cancel decisions,
RecoveryService dispatch, retry eligibility, tab presentation, and notification
copy.

## Invariants

1. `presentation/recovery_capture_abort_coordinator.py` imports no PyQt6,
   editor widget, RecoveryService, or concrete recovery job.
2. A stale job identity returns without mutating tracker, channel, session, or
   notification state.
3. A worker-started abort marks the snapshot discarded; a worker-not-started
   abort releases the snapshot without creating a later callback obligation.
4. Matching capture cleanup removes the producer, aborts the channel when one
   exists, cancels the editor capture session, and releases the document
   lifecycle exactly once in the existing order.
5. Failure notification is emitted only when requested and the owner remains
   live; cancellation remains silent.
6. D90 `RecoveryWriteCoordinator` remains a separate writer-callback boundary;
   writer failure and capture failure do not share a hidden policy seam.

## Alternatives considered

- **Leave abort cleanup in MainWindow:** rejected; failure and cancellation
  cleanup had become a mixed lifecycle block that obscured the worker-started
  discard contract.
- **Move capture stepping or QTimer scheduling into the coordinator:**
  rejected; editor positions and event-loop scheduling are presentation policy.
- **Reuse RecoveryWriteCoordinator for capture abort:** rejected; writer
  callback classification and producer cancellation have different snapshot
  ownership semantics.
- **Make the coordinator know `_RecoveryCaptureJob` or `TextCaptureSession`:**
  rejected; generic adapters preserve Qt/editor independence.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Herschel the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Parfit the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for stale identity suppression, worker-started
  discard versus pre-worker release, channel/session cleanup, document release,
  live-owner notification, and separation from D90 writer failure handling.
- Simplification assessment: the former cleanup block is represented once by a
  generic coordinator while Qt scheduling and retry policy remain local. No
  further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D91-RECOVERY-CAPTURE-ABORT-QT-FREE-BOUNDARY-PROBE=PASS` covers the import
  boundary, MainWindow wiring, and removal of lifecycle policy from the wrapper.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D91 handoff.
- Native editor capture/session timing, channel backpressure, worker
  interleaving, accessibility, DPI, font metrics, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
