# ADR-0106: Recovery-scan coordinator boundary

- **Status:** accepted-with-limits; D81 / ARCH-56 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained the completion side of recovery inventory scanning:
stale-job release, candidate-shape validation, empty/manual feedback,
failure projection, prompt dispatch, and startup continuation. It did not own
the recovery service's scan implementation or the user's restore/discard/later
decision.

## Decision

Extract result classification into the Qt-free `RecoveryScanCoordinator`. It
receives the existing `RecoveryScanTracker`, a typed candidate prompt
callback, a session snapshot getter, a startup continuation callback, and a
typed notification sink. It preserves the scan job identity, candidate
validation, manual/startup empty behavior, invalid-inventory failure, worker
failure, candidate dispatch, and startup continuation.

Keep MainWindow responsible for RecoveryService and TaskRunner dispatch,
RecoveryPromptSurface, restore/discard/later decisions, session/workspace/tab
policy, and close behavior. No recovery persistence, document restore, or
filesystem policy moves.

## Invariants

1. `presentation/recovery_scan_coordinator.py` imports no PyQt6 or widget type.
2. Only the current `RecoveryScanJob` can be finished; stale callbacks return
   without notification or continuation.
3. Inventory must remain a tuple of `RecoveryCandidate` values.
4. Manual empty scans remain informational; startup empty scans continue
   silently into session restore.
5. Invalid inventory and worker failure retain error notification and startup
   continuation behavior.
6. The coordinator never chooses restore/discard/later and never owns a
   RecoveryService or close policy.

## Alternatives considered

- **Leave callbacks in MainWindow:** rejected; inventory result classification
  is a focused framework-neutral lifecycle boundary.
- **Move RecoveryPromptSurface or decisions:** rejected; user interaction and
  restore policy remain presentation/application policy.
- **Move RecoveryScanTracker ownership:** rejected; MainWindow must expose its
  in-flight state to close guards and retain scan lifecycle composition.
- **Add a recovery state machine:** rejected; the existing tracker already
  owns identity and startup context.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Meitner the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Ohm the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review: PASS for job identity, result validation, manual vs
  startup behavior, prompt ownership, and close-policy retention.
- Simplification assessment: three MainWindow result callbacks become one
  focused coordinator with one shared startup-failure path; explicit callback
  seams remain to avoid moving Qt or recovery policy. No further safe reduction
  was identified.

## Verification target and limits

- `D81-RECOVERY-SCAN-BOUNDARY-PROBE=PASS` covers result callbacks, stale
  finish, candidate/prompt ownership, startup continuation, and close guards.
- `D81-RECOVERY-SCAN-COORDINATOR-QT-FREE-PROBE=PASS` confirms importing the
  coordinator through the bare Python path does not import PyQt6.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D81 handoff.
- Native Qt prompt rendering, callback interleaving, recovery interaction,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
