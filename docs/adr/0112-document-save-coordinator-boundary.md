# ADR-0112: Document-save coordinator boundary

- **Status:** accepted-with-limits; D87 / ARCH-62 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained asynchronous document-save completion handling that
combined generic operation completion, tab liveness, editor read-only
restoration, `DocumentState` validation, error projection, and the valid saved
state policy. The latter includes language/title projection, recovery cleanup,
events, notifications, session persistence, and an optional close continuation.

## Decision

Extract the lifecycle and result classification into the Qt-free
`DocumentSaveCoordinator[TabT]`. It receives an opaque tab type, generic
completion guard, tab-liveness predicate, read-only projection, valid-result
policy callback, and error projection.

The coordinator owns stale suppression, live-tab gating, read-only release,
`DocumentState` validation, and invalid/failure error projection. MainWindow
retains state replacement, language/title refresh, recovery-snapshot cleanup,
`DocumentSaved`, success notification, session-save scheduling, and `after`
continuation policy.

## Invariants

1. `presentation/document_save_coordinator.py` imports no PyQt6 and exposes no
   concrete tab, editor, service, filesystem, or event-bus capability.
2. A stale callback returns before checking tab liveness or changing
   read-only state.
3. A non-live tab callback returns after generic completion and does not touch
   an already-removed editor.
4. A live current result restores editability before validating
   `DocumentState`; invalid results preserve the existing Save failed error.
5. A live worker failure restores editability and preserves the existing Save
   failed error; stale/non-live failures do not project an error.
6. `after` is passed only to valid saved-result policy; MainWindow remains the
   owner of state, language, title, recovery, event, notification, and
   persistence consequences.

## Alternatives considered

- **Keep `_on_saved` and `_on_save_failed` in MainWindow:** rejected; shared
  liveness/read-only/result lifecycle is a focused framework-neutral boundary.
- **Create one generic document-operation abstraction for open and save:**
  rejected; save has tab liveness and editor mutability semantics that do not
  match open/session-restore callbacks.
- **Move state replacement, language, recovery, or event policy:** rejected;
  those are application/editor consequences and would couple the coordinator
  to the shell.
- **Use `_DocumentTab` directly in the coordinator:** rejected; generic
  `TabT` keeps the boundary reusable and prevents widget coupling.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Kuhn the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Averroes the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for stale ordering, tab liveness, read-only
  release, typed result validation, exact error projection, and retention of
  valid save policy.
- Simplification assessment: success/failure lifecycle is one generic opaque
  tab boundary, while the after-continuation and document consequences stay
  explicit in MainWindow. Combining open/save or moving editor policy would
  increase coupling. No further safe behavior-preserving reduction was
  identified.

## Verification target and limits

- `D87-DOCUMENT-SAVE-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free import
  boundary, callback wiring, old callback removal, and retained save policy.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D87 handoff.
- Native editor mutability, tab-close interleaving, save timing, accessibility,
  DPI, font metrics, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release owner evidence remain
  unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
