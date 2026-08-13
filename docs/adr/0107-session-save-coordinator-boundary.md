# ADR-0107: Session-save coordinator boundary

- **Status:** accepted-with-limits; D82 / ARCH-57 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained the completion side of session persistence: matching
save callbacks to `SessionSaveTracker`, projecting invalid results and worker
failures, and draining the latest queued request. The shell still owns the
session service, debounce timer, snapshot capture, TaskRunner dispatch, startup
barrier, and close policy.

## Decision

Extract completion classification into the Qt-free
`SessionSaveCoordinator`. It receives the existing tracker, a callback for
draining the next request, and the typed notification sink. It preserves stale
callback suppression, invalid-result notification, failure notification, and
latest-request draining.

Keep MainWindow responsible for `SessionService`, snapshot capture and
debounce, `QTimer`, `TaskRunner`, operation-ID allocation, startup gating,
close guards, and persistence policy. No session format, store, service, or
application contract moves.

## Invariants

1. `presentation/session_save_coordinator.py` imports no PyQt6 or widget type.
2. A stale completion returns without notification or queue draining.
3. A matching invalid result keeps the existing error text and drains the
   next queued request.
4. A matching worker failure keeps the previous-manifest error text and drains
   the next queued request; a stale failure does neither.
5. Callback argument order remains `(result, operation_id)` and
   `(error, operation_id)` at the TaskRunner boundary.
6. The coordinator never owns a timer, `SessionService`, snapshot capture,
   close policy, or session persistence format.

## Alternatives considered

- **Leave callbacks in MainWindow:** rejected; callback classification is a
  focused framework-neutral lifecycle boundary.
- **Move `_drain_session_save`:** rejected; it owns the session service,
  snapshot request state, TaskRunner dispatch, and startup/close policy.
- **Move `SessionSaveTracker`:** rejected; MainWindow must retain its
  in-flight state for close guards and composition, while the coordinator only
  consumes its classification contract.
- **Add a save state machine:** rejected; the existing tracker already owns
  latest-wins and single-flight state.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Noether the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Kierkegaard the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for stale/current identity, invalid/failure
  notifications, callback parameter order, queue draining, Qt-free imports,
  and retention of persistence/close policy.
- Simplification assessment: two MainWindow completion callbacks become one
  focused coordinator while the existing tracker and drain seam remain
  explicit. No further behavior-preserving reduction was identified.

## Verification target and limits

- `D82-SESSION-SAVE-BOUNDARY-PROBE=PASS` covers callback wiring, preserved
  persistence/close ownership, stale/invalid/failure message contracts, and
  removal of the old callbacks.
- `D82-SESSION-SAVE-QT-FREE-PROBE=PASS` confirms the coordinator imports
  without PyQt6 through the bare Python path.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D82 handoff.
- Native Qt callback timing, QTimer interleavings, persistence durability,
  runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
