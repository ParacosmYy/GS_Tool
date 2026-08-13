# ADR-0105: Session-load coordinator boundary

- **Status:** accepted-with-limits; D80 / ARCH-55 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained the result classification for the application session
load operation. The callbacks normalized absent/invalid/failing loads to
`DEFAULT_SESSION`, updated both persistence/restore baselines, notified the
user when the manifest could not be read, and continued the recovery-first
startup flow. They did not own the session service or workspace/tab policy.

## Decision

Extract the callback classification into the Qt-free
`SessionLoadCoordinator`. It receives typed setters for the
`SessionSaveTracker` and `SessionRestoreTracker` baselines, a callback that
continues the startup recovery scan, and a typed notification sink. It keeps
the existing callback signature compatible with `TaskRunner` while ignoring
the operation ID exactly as the previous MainWindow callbacks did.

Keep MainWindow responsible for `SessionService` and `TaskRunner` dispatch,
the startup barrier, tracker composition, recovery/workspace/tab restoration,
and close policy. No persistence format, session service behavior, recovery
policy, or document opening behavior changes.

## Invariants

1. `presentation/session_load_coordinator.py` imports no PyQt6 or widget type.
2. Absent loads use `DEFAULT_SESSION` without the invalid-manifest error;
   invalid/non-typed/failed loads use the existing error notification.
3. A valid snapshot and the fallback baseline update last-saved then restore
   state in the same order as before recovery scanning continues.
4. MainWindow retains session service/TaskRunner ownership, startup barrier,
   workspace/tab policy, and close guards.
5. No new stale guard is introduced because the original load callbacks did
   not classify operation IDs; the callback shape remains compatible.

## Alternatives considered

- **Leave load callbacks in MainWindow:** rejected; result classification is a
  focused Qt-free boundary with no widget dependency.
- **Move SessionService or TaskRunner into the coordinator:** rejected;
  persistence and worker lifetime remain application/shell responsibilities.
- **Move workspace, recovery, or tab restore policy:** rejected; the slice
  only projects the load baseline and starts the existing recovery stage.
- **Add a new session state machine:** rejected; `SessionRestoreTracker` and
  `SessionSaveTracker` already own their respective state boundaries.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Gibbs the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Tesla the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review: PASS for load-state semantics, baseline ordering,
  callback compatibility, and policy retention.
- Simplification assessment: removing two MainWindow callbacks and centralizing
  their shared fallback path reduces duplication while retaining explicit
  typed setters/callbacks. No further safe reduction was identified.

## Verification target and limits

- `D80-SESSION-LOAD-BOUNDARY-PROBE=PASS` covers callback removal/wiring,
  baseline/recovery contracts, composition order, and close-gate retention.
- `D80-SESSION-LOAD-COORDINATOR-QT-FREE-PROBE=PASS` confirms importing the
  coordinator through the bare Python path does not import PyQt6.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D80 handoff.
- Native Qt callback timing, rendering, accessibility, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
