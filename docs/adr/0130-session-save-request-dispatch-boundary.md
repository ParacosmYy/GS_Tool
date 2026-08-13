# ADR-0130: Session-save request and dispatch boundary

- **Status:** accepted-with-limits; D103/ARCH-77 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`SessionSaveTracker` already made latest-wins state and operation identity
framework-neutral, and `SessionSaveCoordinator` classified completion
callbacks. MainWindow nevertheless retained the adjacent request/dispatch
sequence: capture a snapshot after debounce, request it, allocate an operation
ID, begin the tracker, submit to `TaskRunner`, and drain after completion. This
split made the full persistence lifecycle harder to review and left duplicate
eligibility/queue decisions in the composition root.

## Decision

Complete the existing `SessionSaveCoordinator` boundary. It receives explicit
callbacks for save eligibility, snapshot capture, operation-ID allocation, and
concrete TaskRunner submission. It owns only this framework-neutral sequence:

1. `request_latest()` checks eligibility, captures the latest snapshot, and
   queues it through `SessionSaveTracker`.
2. `drain()` starts only when a request exists and no save is in flight,
   binds the next operation ID, and invokes the submission callback.
3. `complete()` and `fail()` preserve the existing tracker classification,
   notification, stale suppression, and drain behavior.

MainWindow retains QTimer debounce and immediate-trigger policy,
`SessionSnapshot` capture callbacks, `SessionService`, `TaskRunner`, concrete
operation completion callbacks, startup-restore admission, notifications, and
close policy. No new coordinator or parallel session contract is introduced.

## Invariants

1. The session schema, `SessionService.normalize_session()`, `SessionStore`,
   operation-ID allocation, 250 ms timer, and TaskRunner queued-completion
   boundary are unchanged.
2. A matching valid callback updates the saved baseline; an invalid callback
   reports an error without advancing the baseline; a matching failure keeps
   the previous baseline and reports an error.
3. A stale callback cannot release or drain the current operation. A queued
   latest snapshot is drained only after the current operation is consumed.
4. No Qt, `TaskRunner`, `SessionService`, filesystem, or widget type enters
   `session_save_coordinator.py`; all concrete effects remain callbacks.
5. Startup restore and no-service guards remain explicit and close-readiness
   still observes the same `SessionSaveTracker.inflight` state.

## Alternatives considered

- **Leave request/dispatch in MainWindow:** rejected; it leaves the existing
  completion coordinator boundary incomplete and duplicates the tracker
  lifecycle across two owners.
- **Create a second `SessionSaveRequestCoordinator`:** rejected; it would
  split one lifecycle and create parallel contracts around the same tracker.
- **Move QTimer or SessionService into the coordinator:** rejected; it would
  introduce Qt/application/infrastructure ownership into a framework-neutral
  presentation contract.
- **Change SessionSaveTracker semantics:** rejected; the tracker already owns
  the correct latest-wins and stale identity invariants.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation architecture. The D103
coordinator itself is Qt-free and relies only on project contracts and Python
callables; no MCU, embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver,
boot, Flash/NVM, power-control, motor-control, or manufacturer requirement is
applicable. The mandatory embedded assurance gate is therefore recorded as
N/A. Public CloudWeGo material remains an engineering reference only; no
private ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect: Wegener the 3rd / Luna max; two bounded read-only waits returned
  `NO_CONCLUSION`, then the agent was closed. No child architecture PASS is
  claimed.
- Independent review: Chandrasekhar the 3rd / Luna max; two bounded read-only
  waits returned `NO_CONCLUSION`, then the agent was closed. No independent
  PASS is claimed.
- Parent source review: PASS for callback ordering, tracker semantics,
  initialization/lifecycle, Qt-free imports, MainWindow ownership, and
  preservation of stale/invalid/failure paths.
- Simplification assessment: PASS. The existing coordinator is extended
  instead of adding a parallel request coordinator; MainWindow loses only the
  duplicate queue/drain methods. No further safe reduction was identified.

## Verification target and limits

- D103 source/order, Qt-free dependency, and inline behavior probes must pass.
- Compileall, Ruff, format, package identity, handoff, repository,
  traceability, and expected release NO-GO evidence are required.
- Native Qt startup, worker timing, filesystem durability, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner evidence remain unrun or open under the active authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run.
