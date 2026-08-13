# ADR-0111: Document-open coordinator boundary

- **Status:** accepted-with-limits; D86 / ARCH-61 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained asynchronous document-open completion handling for both
ordinary opens and ordered session restoration. The callbacks mixed generic
operation completion, session-restore binding consumption, `OpenedDocument`
validation, invalid-result/failure classification, and valid tab/editor/event
policy.

## Decision

Extract the callback classification into the Qt-free
`DocumentOpenCoordinator`. It receives the existing generic completion guard,
session-restore identity/consumption seams, valid-open policy callback,
session continuation, error projection, and notification sink.

The coordinator owns only stale suppression, session binding consumption,
`OpenedDocument` validation, ordinary-open failure projection, and
session-restore invalid/failure continuation. MainWindow retains duplicate-tab
policy, editor/tab construction, line and cursor positioning, event
publication, success notification, and the save/close/application policy.

## Invariants

1. `presentation/document_open_coordinator.py` imports no PyQt6 and has no tab,
   editor, filesystem, service, or event-bus capability.
2. A stale callback returns before consuming a session binding or projecting
   any result, error, notification, or continuation.
3. A current callback consumes the matching session binding exactly once when
   the operation is a session restore.
4. An ordinary invalid result preserves `Open failed` and the existing invalid
   document-service error message; a session invalid result warns and
   continues the restore queue.
5. An ordinary worker failure preserves `Operation failed`; a session worker
   failure warns only when a bound document exists and continues the queue.
6. Valid `OpenedDocument` handling remains in MainWindow, including line
   number, duplicate tab, editor creation, cursor, restored-tab, event, and
   success-notification policy.

## Alternatives considered

- **Keep `_on_opened` and both failure callbacks in MainWindow:** rejected;
  completion classification and session binding consumption are one focused
  framework-neutral lifecycle.
- **Move duplicate-tab or editor creation into the coordinator:** rejected;
  that would couple the boundary to Qt widgets and application tab policy.
- **Combine document save into this slice:** rejected; save requires tab
  liveness, read-only restoration, language projection, recovery cleanup, and
  save continuation, so it is a separate bounded contract.
- **Move `SessionRestoreTracker` itself:** rejected; its ordered restore state
  remains the existing typed tracker boundary.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Copernicus the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Hubble the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for stale ordering, session binding consumption,
  ordinary/session invalid results, ordinary/session failures, line-number
  forwarding, and retention of tab/editor/event policy.
- Simplification assessment: three open completion/failure routes become one
  Qt-free coordinator while the valid document policy remains in one focused
  MainWindow method. Save callbacks remain separate because their tab-liveness
  and editor-state policy is materially different. No further safe
  behavior-preserving reduction was identified.

## Verification target and limits

- `D86-DOCUMENT-OPEN-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free import
  boundary, callback wiring, old callback removal, session-binding ownership,
  and retained tab/save seams.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D86 handoff.
- Native editor/tab rendering, callback timing, session restore interleaving,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
