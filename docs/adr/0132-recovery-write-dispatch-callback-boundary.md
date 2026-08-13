# ADR-0132: Recovery-write dispatch callback boundary

- **Status:** accepted-with-limits; D105/ARCH-78 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The streamed-channel and completed-chunk recovery paths both submitted a
`TaskRunner` operation from MainWindow and repeated the same success/failure
closures. Those closures carried the same owner, content-version, and
snapshot identity into `RecoveryWriteCoordinator.complete()` and `.fail()`.
The duplication made callback ordering and lifecycle ownership harder to
review, while the concrete payloads correctly remained MainWindow concerns.

## Decision

Extend the existing Qt-free `RecoveryWriteCoordinator` with a typed
`submit(...)` method. It accepts an opaque write operation, operation ID,
owner/content-version/snapshot identity, and a generic dispatcher callable. It
creates the success/failure callbacks that call the existing lifecycle
classification methods, then invokes the dispatcher. MainWindow continues to
allocate the operation ID, choose the completed tuple or bounded channel,
capture the concrete RecoveryService, and pass the existing TaskRunner
dispatcher.

The dispatch callable is deliberately structural: no `TaskRunner` import or
Qt type enters the coordinator. If the concrete dispatcher raises before
accepting the operation, the exception still propagates to the original caller
with the same pre-worker cleanup handling. Once accepted, callback semantics
remain the existing complete/fail path.

## Invariants

1. Recovery payload construction, `channel.consume()` timing, operation-ID
   allocation, `RecoveryService`, `TaskRunner`, capture, notification, delete,
   persistence, and close policy remain in MainWindow.
2. Success callbacks still release document/write lifecycle before existing
   recovery projection; failures still abort a bound capture, release write
   lifecycle, and project failure through the existing coordinator chain.
3. The callback operation ID is accepted for dispatcher compatibility but
   identity classification remains the existing snapshot/owner contract; no
   new stale or discard policy is introduced.
4. Dispatcher exceptions propagate synchronously; no callback is synthesized
   and no lifecycle state is silently released by the new method.
5. `recovery_write_coordinator.py` remains free of Qt, TaskRunner,
   RecoveryService, filesystem, and widget dependencies.

## Alternatives considered

- **Leave the duplicate closures in MainWindow:** rejected; it keeps two
  sources for the same callback binding and weakens reviewability.
- **Add a new `RecoveryWriteDispatchCoordinator`:** rejected; the existing
  `RecoveryWriteCoordinator` already owns the exact completion lifecycle, so a
  second coordinator would split one contract.
- **Move TaskRunner or RecoveryService into the coordinator:** rejected; it
  would violate the Qt-free boundary and move concrete infrastructure policy.
- **Use a MainWindow-only helper:** rejected; it would remove text duplication
  but leave lifecycle callback binding outside the existing owner.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is therefore `N/A`. Public CloudWeGo material remains an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made.

## Review and simplification

- Architect: Russell the 3rd / Luna max; the bounded read-only wait timed out
  and the agent was closed. Status is `NO_CONCLUSION`; no child architecture
  PASS is claimed.
- Independent review: Kepler the 3rd / Luna max; the bounded read-only wait
  timed out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for dispatcher signature, callback ordering,
  exception propagation, recovery lifecycle preservation, and dependency
  direction.
- Simplification assessment: PASS. The existing lifecycle coordinator is
  extended instead of adding a helper/coordinator pair; duplicated callback
  closures are removed and no new state is introduced.

## Verification target and limits

- Required: source/dependency probe, inline dispatch success/failure probe,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, and expected release NO-GO evidence.
- Not proven: native Qt worker timing, channel backpressure, filesystem
  durability, crash/restart recovery, screenshots, clean-machine behavior,
  signing, installer, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
