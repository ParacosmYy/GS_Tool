# ADR-0134: Recovery-delete dispatch callback boundary

- **Status:** accepted-with-limits; D107/ARCH-79 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`RecoveryDeleteCoordinator` already owns the Qt-free delete lifecycle:
tracker release, owner snapshot clearing, success/error notification, and
pending-delete draining. MainWindow still repeated the same callback closures
when it submitted `RecoveryService.delete_snapshot()` to `TaskRunner`. The
closures carried the snapshot identity, optional tab owner, and success
message that the coordinator already needs.

## Decision

Extend the existing `RecoveryDeleteCoordinator` with typed
`DeleteOperation`, `DeleteSuccess`, `DeleteFailure`, and `DeleteDispatcher`
contracts plus a keyword-only `submit(...)` method. The method binds the
snapshot identity, owner, and success message to the existing `complete()` and
`fail()` methods, then invokes a structural dispatcher supplied by MainWindow.
MainWindow now passes the concrete delete operation and `TaskRunner.submit`
while retaining operation-ID allocation and all service/policy ownership.

The dispatcher remains structural. The coordinator imports neither Qt nor
`TaskRunner`, so the callback boundary is reusable by another worker adapter
without changing the lifecycle owner. If dispatch raises synchronously, the
exception propagates exactly as before; the coordinator does not synthesize a
callback or release tracker state.

## Invariants

1. `RecoveryCaptureTracker.request_delete()` admission, operation-ID
   allocation, `RecoveryService`, `TaskRunner`, persistence, notification,
   recovery/write interaction, and close policy remain in MainWindow.
2. Matching success still releases delete state, clears the matching owner
   snapshot, emits the existing success message, and drains one pending delete
   in the same order.
3. Matching failure still releases delete state and emits the existing cleanup
   error without clearing the owner or dropping a pending request.
4. The operation ID is retained for dispatcher compatibility and callback
   signature compatibility; lifecycle identity remains the existing snapshot
   contract.
5. `recovery_delete_coordinator.py` remains free of Qt, TaskRunner,
   RecoveryService, filesystem, and widget dependencies.

## Alternatives considered

- **Keep the duplicate MainWindow closures:** rejected; it leaves callback
  binding split from the lifecycle owner and repeats identity capture.
- **Create a second delete-dispatch coordinator:** rejected; the existing
  coordinator already owns the exact delete completion lifecycle.
- **Move RecoveryService or TaskRunner into the coordinator:** rejected; that
  would reverse dependency direction and mix concrete infrastructure with a
  Qt-free contract.
- **Generalize all MainWindow task submissions at once:** rejected; it would
  widen the slice and obscure distinct service/policy boundaries.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the required embedded workflow and simplifier were
reviewed for applicability and no embedded source was changed. Public
CloudWeGo material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Boole the 3rd / Luna max; bounded read-only wait timed out and
  the agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS
  is claimed.
- Independent review: Locke the 3rd / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for typed dispatcher shape, callback ordering,
  tracker/owner/notification behavior, synchronous exception propagation, and
  dependency direction.
- Simplification assessment: PASS. The existing lifecycle coordinator is
  extended rather than paired with a new helper; duplicate closures and no
  new state are removed.

## Verification target and limits

- Required and authorized here: source/dependency probe, inline production
  class success/failure/pending/exception dispatch probe, compileall, Ruff,
  format, package identity, handoff/register/index synchronization, and
  expected release NO-GO evidence.
- Not proven: native Qt worker timing, filesystem durability, crash/restart
  recovery, QApplication startup, screenshots, accessibility/DPI/font
  rendering, clean-machine/cross-machine behavior, signing, installer,
  legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
