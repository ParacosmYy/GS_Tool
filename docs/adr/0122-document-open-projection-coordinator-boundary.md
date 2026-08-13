# ADR-0122: Document-open projection coordinator boundary

- **Status:** accepted-with-limits; D97 / ARCH-71 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D86 moved asynchronous open callback classification into
`DocumentOpenCoordinator`, while MainWindow's valid-result callback still
combined duplicate path handling, tab projection, optional line/cursor
positioning, session-restored-tab recording, event publication, success
notification, and session-restore continuation. That left one use-case outcome
with classification in one module and valid projection sequencing in a large
Qt shell method.

## Decision

Extract the valid-result sequence into the Qt-free generic
`DocumentOpenProjectionCoordinator[TabT]`. It receives an `OpenedDocument`,
optional line number, session-restore flag, and optional `SessionDocument`,
then uses explicit callbacks for existing-tab lookup, duplicate error,
tab creation, line/cursor projection, restored-tab recording, event
publication, success notification, and restore continuation.

`DocumentOpenCoordinator` remains the owner of stale suppression, result
validation, restore-binding consumption, and ordinary/session failure
classification. MainWindow remains the composition root and retains concrete
DocumentService/TaskRunner wiring, editor/tab construction, close policy, and
all callback implementations.

## Invariants

1. `document_open_projection_coordinator.py` imports no PyQt6, editor widget,
   tab surface, MainWindow, EventBus, filesystem service, or persistence API.
2. Ordinary duplicate opens show the existing error and produce no tab, event,
   notification, or restore continuation.
3. Session duplicate opens record the existing tab and continue the restore
   queue without creating a second tab or publishing an open event.
4. A new result is added before line/cursor projection; a session cursor is
   recorded before `DocumentOpened`, success notification, and continuation.
5. Untitled results preserve the existing `document` notification fallback;
   path identity and line/cursor values are passed through unchanged.
6. D86 stale/invalid/failure ordering and D95 tab-creation ordering remain
   unchanged; no new async or document state model is introduced.

## Alternatives considered

- **Keep the entire valid callback in MainWindow:** rejected; it leaves
  duplicate/restore projection sequencing coupled to a widget shell and keeps
  the D86 boundary incomplete.
- **Expand `DocumentOpenCoordinator` to own Qt-facing projection:** rejected;
  it would mix callback classification with a wider result policy and weaken
  the existing Qt-free contract.
- **Create a generic document-operation coordinator for open and save:**
  rejected; save has tab-liveness, read-only, language, recovery, and
  continuation semantics that do not share this projection contract.
- **Move concrete editor/event implementations into the new module:**
  rejected; explicit callbacks keep dependency direction and composition-root
  ownership visible.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Leibniz the 3rd / Luna max; bounded read-only window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Dewey the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for ordinary/session duplicate behavior,
  projection order, Qt-free dependencies, and retained D86/D95 boundaries.
- Simplification assessment: PASS; the extraction removes one valid-open
  policy block without introducing a second state model or generic operation
  framework. No further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D97-DOCUMENT-OPEN-PROJECTION-PROBE=PASS` covers ordinary, session, new-tab,
  line/cursor, duplicate, and restore-continuation ordering.
- `D97-QT-FREE-WIRING-PROBE=PASS` covers forbidden dependency strings, removal
  of the old MainWindow method, and direct D86 wiring.
- Compileall, Ruff, format, package identity, traceability, handoff,
  repository, no-process, and expected release NO-GO evidence are recorded in
  the D97 handoff.
- Native editor/tab/session timing, accessibility, DPI, font metrics, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
