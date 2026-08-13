# ADR-0123: Document-save projection coordinator boundary

- **Status:** accepted-with-limits; D98 / ARCH-72 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D87 moved asynchronous save completion classification into
`DocumentSaveCoordinator`, but the valid-result callback still kept the full
success projection in `MainWindow`: state and clean-editor projection,
language/title refresh, recovery cleanup, `DocumentSaved`, success feedback,
session persistence, and the optional close/save continuation. The stale,
invalid, and failure boundary was explicit while the valid branch remained a
large shell-owned sequence.

## Decision

Extract that valid branch into the Qt-free generic
`DocumentSaveProjectionCoordinator[TabT]`. Its contract receives a
`DocumentState` already validated by D87 for a live tab and invokes explicit
callbacks in the existing observable order:

1. apply the saved state and clean editor state;
2. refresh language and tab title;
3. clear the recovery snapshot;
4. publish `DocumentSaved` and notify success;
5. request session persistence; and
6. invoke the optional `after` continuation last.

`DocumentSaveCoordinator` remains responsible for operation completion and
stale suppression, tab liveness, read-only release, `DocumentState` validation,
and invalid/failure error projection. `MainWindow` remains the composition root
and supplies concrete editor, recovery, event, notification, persistence, and
close-policy callbacks.

## Invariants

1. `document_save_projection_coordinator.py` imports no PyQt6, widget, editor,
   tab-surface, EventBus, filesystem service, or persistence implementation.
2. The new coordinator is called only after D87 has accepted a matching live
   save callback and restored editability; it does not reclassify or repair
   invalid state.
3. Callback ordering and exception propagation remain unchanged from the former
   `MainWindow._apply_saved_document` method; no callback is swallowed or
   retried.
4. The optional continuation runs only after session-save scheduling, exactly
   as before, and a missing continuation is a no-op.
5. No save target, encoding, conflict, document service, recovery policy,
   session schema, event contract, or close guard changes.

## Alternatives considered

- **Keep the valid sequence in MainWindow:** rejected; it leaves the D87 valid
  branch coupled to the shell and keeps post-save ordering hidden in a large
  composition-root method.
- **Expand DocumentSaveCoordinator with every success callback:** rejected;
  it would combine callback classification and result projection, weakening the
  existing D87 ownership split.
- **Create one generic coordinator for all document operations:** rejected;
  open, save, recovery, and close have distinct contracts and failure semantics;
  a universal dispatcher would add indirection without removing policy.
- **Move editor/event/persistence implementations into the new module:**
  rejected; explicit callbacks preserve dependency direction and composition
  root ownership.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance gate is recorded as N/A for this change.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Popper the 3rd / Luna max; two bounded read-only waits returned
  `NO_CONCLUSION`, then the agent was closed. No child architecture PASS is
  claimed.
- Independent review: Helmholtz the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No independent PASS is
  claimed.
- Parent source review: PASS for exact projection ordering, preserved D87
  classification/liveness/error semantics, Qt-free imports, and direct wiring.
- Simplification assessment: PASS. The extraction removes the valid-save
  policy block from MainWindow while keeping one explicit, typed boundary; it
  does not introduce a service locator, shared state model, or generic
  document-operation framework. No further safe behavior-preserving reduction
  was identified.

## Verification target and limits

- `D98-SAVE-PROJECTION-SOURCE-PROBE=PASS` covers the Qt-free source boundary,
  direct wiring, and removal of the old MainWindow method.
- `D98-SAVE-PROJECTION-ORDER-PROBE=PASS` covers state/language/title/recovery/
  event/notification/session/continuation order and path pass-through.
- Compileall, Ruff, format, package identity, handoff, repository, no-process,
  traceability, and expected release NO-GO evidence are recorded in the D98
  handoff.
- Native editor/save timing, accessibility, DPI, fonts, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
