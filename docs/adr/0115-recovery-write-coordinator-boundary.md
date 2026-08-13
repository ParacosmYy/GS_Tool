# ADR-0115: Recovery-write coordinator boundary

- **Status:** accepted-with-limits; D90 / ARCH-65 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

Recovery snapshot writers can run in two modes: a completed UI capture passed
as chunks, or a bounded channel consumed by a worker while capture continues.
Their callbacks previously mixed discarded-snapshot consumption, capture
failure cleanup, document/write tracker release, pending-delete drain, and
tab/content projection.

## Decision

Extract the common write lifecycle into the Qt-free generic
`RecoveryWriteCoordinator[JobT, OwnerT]`. It receives the existing
`RecoveryCaptureTracker`, owner document identity, capture-abort seam,
write-finish/pending-delete seam, and saved/failed policy callbacks.

The coordinator owns discarded classification, failed capture abort, document
release, write release, and callback ordering. MainWindow retains tab liveness,
dirty/content-version/snapshot identity decisions, delete scheduling, and
notifications.

## Invariants

1. `presentation/recovery_write_coordinator.py` imports no PyQt6, editor
   widget, RecoveryService, or concrete recovery job.
2. Success ordering is discarded consumption → document release → write finish
   (including pending-delete drain) → saved policy projection.
3. Failure ordering is discarded consumption → matching capture abort →
   document release → write finish → failed policy projection.
4. The worker operation ID remains an opaque TaskRunner delivery detail; these
   recovery writes do not enter generic busy operation policy.
5. MainWindow retains tab liveness, dirty/content-version checks, snapshot
   matching, delete policy, and user-facing notification decisions.

## Alternatives considered

- **Leave both writer callback bodies in MainWindow:** rejected; the two writer
  modes share one tracker lifecycle and ordering contract.
- **Move tab dirty/content-version or delete policy into the coordinator:**
  rejected; those are document/recovery application consequences.
- **Make recovery writes generic busy operations:** rejected; existing
  background autosave intentionally uses pending-worker status without taking
  the foreground operation lock.
- **Import `_RecoveryCaptureJob` or EditorWidget:** rejected; generic job/owner
  seams preserve Qt-free composition.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Peirce the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Banach the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for success/failure ordering, discarded/capture
  cleanup, document/write release, pending-delete continuation, and retention
  of tab/content/notification policy.
- Simplification assessment: both writer paths now share one typed lifecycle
  without changing background operation semantics. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D90-RECOVERY-WRITE-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free import
  boundary, callback wiring, tracker lifecycle ownership, old callback removal,
  and retained recovery policy.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D90 handoff.
- Native capture/write interleaving, channel backpressure, delete timing,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
