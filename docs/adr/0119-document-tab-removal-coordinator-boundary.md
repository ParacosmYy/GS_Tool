# ADR-0119: Document-tab removal coordinator boundary

- **Status:** accepted-with-limits; D94 / ARCH-69 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._close_tab` correctly owns busy/startup guards and dirty-document
save confirmation, but the final removal path still mixed recovery capture
cancellation, snapshot cleanup, tab projection, editor teardown, close-event
publication, session persistence, and empty-tab fallback.

## Decision

Extract the approved-removal finalization into the Qt-free generic
`DocumentTabRemovalCoordinator[TabT, CaptureT]`. It receives explicit generic
callbacks for tab liveness, recovery capture/snapshot cleanup, tab projection,
editor teardown, event projection, session save, and empty-tab fallback.

MainWindow retains the decision to close, dirty/save/cancel policy, tab index
validation, recovery service behavior, and all user-facing close messaging.

## Invariants

1. The coordinator imports no PyQt6, editor widget, EventBus, RecoveryService,
   or concrete tab/capture type.
2. A non-live tab returns without mutating recovery, tab, editor, event, or
   session state.
3. A live tab preserves the existing order: cancel capture → clear recovery
   snapshot → remove tab projection → delete editor later → publish
   `DocumentClosed` → request session save → ensure an initial document only
   when no tab remains.
4. Close eligibility and save-before-close decisions remain in MainWindow.
5. Recovery delete admission/dispatch remains behind the existing recovery
   seams; this coordinator only invokes snapshot cleanup.

## Alternatives considered

- **Leave `_remove_tab` in MainWindow:** rejected; the approved removal
  lifecycle still had multiple unrelated reasons to change.
- **Move dirty/save-confirm decisions into the coordinator:** rejected; those
  are user-facing close policy and can require asynchronous Save.
- **Make the coordinator own Qt widgets or EventBus:** rejected; generic
  callbacks preserve presentation/application boundaries.
- **Merge tab removal with `DocumentTabSurface`:** rejected; the surface owns
  widget projection, not recovery, event, persistence, or initial-document
  policy.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Lagrange the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Harvey the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for close-policy retention, liveness guard,
  recovery cleanup, projection/editor/event/session order, and empty-tab
  fallback.
- Simplification assessment: the finalization lifecycle is represented once by
  a generic coordinator without changing close semantics or adding a framework.
  No further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D94-DOCUMENT-TAB-REMOVAL-QT-FREE-BOUNDARY-PROBE=PASS` covers the import
  boundary, MainWindow delegation, and removal of finalization policy from the
  shell method.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D94 handoff.
- Native close/save timing, recovery delete timing, editor teardown,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
