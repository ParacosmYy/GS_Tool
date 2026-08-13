# ADR-0147: Session-save dispatch callback boundary

- **Status:** accepted-with-limits; D116 / ARCH-90 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

SessionSaveCoordinator already owned latest-wins admission, tracker identity,
invalid-result classification, failure notification, and queued-drain
behavior. MainWindow still contained `_submit_session_save`, which directly
bound TaskRunner completion and failure callbacks to the coordinator.

## Decision

Add typed SessionSaveOperation, SessionSaveSuccess, SessionSaveFailure, and
SessionSaveDispatcher contracts to the existing Qt-free coordinator. The
coordinator's `submit(...)` binds the admitted operation ID to its existing
`complete(...)` and `fail(...)` methods. `drain()` creates the injected
SessionService operation and submits it through the injected dispatcher.
MainWindow retains the service operation factory, snapshot capture/debounce,
notification, startup restore, persistence, and close policy; it only injects
the existing TaskRunner callable and no longer wires completion/failure
callbacks in a helper.

## Invariants

1. SessionSaveCoordinator remains free of PyQt6, TaskRunner, SessionService,
   filesystem, and widget dependencies.
2. The tracker begins one operation before dispatch; valid, invalid, failure,
   stale, and queued latest-wins outcomes retain their previous ordering.
3. Invalid results and worker failures retain the existing notification text,
   level, and subsequent drain behavior.
4. A synchronous dispatcher exception propagates and leaves the tracker in its
   existing in-flight state; no retry or swallow policy is introduced.
5. MainWindow retains snapshot capture, service/persistence, startup restore,
   notification, debounce, and close-policy ownership.

## Alternatives considered

- **Keep `_submit_session_save` direct runner binding:** rejected; callback
  ownership remains split between the composition root and lifecycle owner.
- **Move SessionService or snapshot capture into the coordinator:** rejected;
  it would couple the Qt-free presentation boundary to persistence policy.
- **Add a generic runner adapter or event bus:** rejected; one typed dispatch
  seam is sufficient and the extra abstraction would widen the change.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Mill the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Curie the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent source review: PASS for tracker-before-dispatch ordering, typed
  binding, invalid/failure notification preservation, service ownership,
  dependency direction, and synchronous exception propagation.
- Simplification assessment: PASS. One coordinator `submit(...)` seam and an
  injected operation factory are the smallest complete change; no generic
  runner adapter or new state owner is needed.

## Verification target and limits

- Authorized evidence includes the D116 session-save lifecycle probe, source/
  dependency probe, compileall, Ruff, format, presentation-contract audit,
  package identity, handoff/register/index synchronization, no-process
  evidence, and expected release NO-GO evidence.
- Not proven: native TaskRunner timing, actual filesystem/session durability,
  QApplication startup, accessibility/DPI/font behavior, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
