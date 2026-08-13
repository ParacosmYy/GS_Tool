# ADR-0117: Session-restore coordinator boundary

- **Status:** accepted-with-limits; D92 / ARCH-67 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The typed `SessionRestoreTracker` already owned ordered restore values,
workspace blocking, recovery deferrals, pending document/open binding, and
restored-tab selection. MainWindow still contained the synchronous stepping
loop that skipped deferred paths, reused already-open tabs, stopped at the next
async open, and finalized startup when all paths were handled.

## Decision

Extract that stepping and completion decision into the Qt-free generic
`SessionRestoreCoordinator[TabT]`. It consumes the existing tracker and
explicit seams for startup activity, tab lookup/projection, initial-document
creation, async open dispatch, deferred-path notification, session finish, and
session-save scheduling.

The coordinator advances synchronously until one asynchronous document open is
required. MainWindow retains workspace service/barrier control, TaskRunner and
document-open callbacks, startup/close state, tab/editor projection, and user
notification wording.

## Invariants

1. `presentation/session_restore_coordinator.py` imports no PyQt6, editor
   widget, TaskRunner, DocumentService, WorkspaceService, or concrete tab.
2. A non-active or workspace-blocked restore is a no-op.
3. Deferred documents are notified and skipped in original order; already-open
   documents are recorded in original order without another open request.
4. The first not-yet-open document is bound to the tracker before the async open
   seam is invoked, and the coordinator stops there for callback continuation.
5. Completion selects the active restored path or first restored tab, creates an
   initial document only when no tab exists, then finishes restore before
   scheduling the session save, matching the prior order.
6. Open-result classification remains in `DocumentOpenCoordinator`; this slice
   does not merge service, TaskRunner, or callback outcome policy.

## Alternatives considered

- **Leave the stepping loop in MainWindow:** rejected; ordered restore state was
  already explicit, but its progression and completion policy remained mixed
  with the Qt shell.
- **Move workspace restore or document opening into this coordinator:**
  rejected; those are service, TaskRunner, and application callback boundaries.
- **Make the coordinator own startup flags or tab widgets:** rejected; generic
  callbacks preserve composition and future multi-window support.
- **Use recursive re-entry for deferred/duplicate paths:** rejected; a local
  loop preserves the same synchronous semantics without unbounded call-stack
  growth.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Erdos the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Heisenberg the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for barrier guards, ordered skip/reuse behavior,
  pending-open stop, completion selection, initial-document fallback, and
  finish/save ordering.
- Simplification assessment: a small loop replaces recursive same-thread
  re-entry while keeping the async open stop and all existing policy seams. No
  further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D92-SESSION-RESTORE-QT-FREE-BOUNDARY-PROBE=PASS` covers the import boundary,
  MainWindow wiring, delegation, and removal of tracker stepping from the
  shell method.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D92 handoff.
- Native session/open callback timing, workspace restore interaction,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
