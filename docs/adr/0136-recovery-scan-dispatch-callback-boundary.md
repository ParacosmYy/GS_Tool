# ADR-0136: Recovery-scan dispatch callback boundary

- **Status:** accepted-with-limits; D109/ARCH-81 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`RecoveryScanCoordinator` already owns recovery inventory validation, scan
tracker completion, candidate prompting, manual empty-result feedback, failure
notification, and startup session continuation. MainWindow still repeated two
closures only to bind one `RecoveryScanJob` before calling `TaskRunner.submit`.

## Decision

Extend the existing Qt-free coordinator with typed `ScanOperation`,
`ScanSuccess`, `ScanFailure`, and `ScanDispatcher` contracts plus a keyword-only
`submit(...)` method. It binds the opaque job to the established `complete()`
and `fail()` methods, then invokes a structural dispatcher supplied by
MainWindow. MainWindow now passes `RecoveryService.scan_candidates` and
`TaskRunner.submit` while retaining operation-ID allocation and all startup or
manual-scan policy.

The dispatcher is structural: no Qt or TaskRunner dependency enters the
coordinator. A synchronous dispatcher exception propagates without invoking a
callback or releasing the tracker, matching the existing pre-worker failure
boundary.

## Invariants

1. `RecoveryScanTracker.begin()` admission, operation IDs, RecoveryService,
   TaskRunner, startup/manual context, notification surface, session restore,
   and close policy remain in MainWindow.
2. Matching success still calls the existing result validation and tracker
   finish path; manual empty scans notify info and startup scans continue
   session restore exactly as before.
3. Matching failure still calls the existing failure projection and startup
   continuation; stale jobs remain suppressed by `RecoveryScanTracker`.
4. The operation ID remains in the generic dispatcher/callback signature for
   TaskRunner compatibility; the lifecycle identity remains the job object.
5. `recovery_scan_coordinator.py` remains free of Qt, TaskRunner,
   RecoveryService, filesystem, and widget dependencies.

## Alternatives considered

- **Keep MainWindow's two closures:** rejected; it duplicates callback binding
  next to a lifecycle owner that already has the complete/fail contract.
- **Create a separate scan-dispatch coordinator:** rejected; it would split
  one inventory lifecycle across two classes.
- **Move RecoveryService or TaskRunner into RecoveryScanCoordinator:**
  rejected; it would violate the Qt-free dependency direction and move startup
  policy out of the composition root.
- **Generalize all remaining MainWindow submissions:** rejected; it would
  broaden this bounded recovery slice and obscure operation-specific contracts.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the embedded workflow and simplifier were reviewed
for applicability and no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Nash the 3rd / Luna max; bounded read-only wait timed out and the
  agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS is
  claimed.
- Independent review: Raman the 3rd / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for job identity, tracker/stale ordering,
  invalid-result and manual/startup behavior, exception propagation, and
  dependency direction.
- Simplification assessment: PASS. The existing coordinator is extended;
  duplicated closures, extra state, and a generic task abstraction are not
  introduced.

## Verification target and limits

- Required and authorized here: source/dependency/order probe, inline success,
  failure, stale, startup-continuation, and exception dispatch probe,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, and expected release NO-GO evidence.
- Not proven: native TaskRunner timing, filesystem/recovery durability,
  QApplication startup, screenshots, accessibility/DPI/font rendering,
  clean-machine/cross-machine behavior, signing, installer, legal, support,
  or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
