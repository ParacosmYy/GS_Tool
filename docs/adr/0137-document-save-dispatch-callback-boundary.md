# ADR-0137: Document-save dispatch callback boundary

- **Status:** accepted-with-limits; D110/ARCH-82 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`DocumentSaveCoordinator` already owns save callback classification: operation
liveness, tab presence, editor read-only release, `DocumentState` validation,
invalid/failure projection, and post-save continuation. MainWindow repeated
success/failure closures only to bind the tab and optional `after` callback
before submitting `DocumentService.save_document()`.

## Decision

Extend the existing Qt-free coordinator with typed `SaveOperation`,
`SaveContinuation`, `SaveSuccess`, `SaveFailure`, and `SaveDispatcher` contracts
plus a keyword-only `submit(...)` method. The method binds the tab and
continuation to the existing `complete()`/`fail()` methods, then invokes a
structural dispatcher supplied by MainWindow.

MainWindow now passes the concrete save operation and `TaskRunner.submit` while
retaining document state/text/path snapshots, read-only transition,
operation-ID allocation, DocumentService, busy/status/notification,
persistence, and close policy. If dispatch raises synchronously, it propagates
without synthesizing a completion or changing coordinator-owned state.

## Invariants

1. Existing operation-ID stale suppression, tab liveness check, read-only
   release, `DocumentState` validation, invalid/failure projection, and
   `after` continuation order are unchanged.
2. The coordinator remains Qt-free and imports neither TaskRunner,
   DocumentService, filesystem, nor widgets.
3. MainWindow keeps save admission, target uniqueness, snapshots, service,
   runner, operation/busy status, persistence, and close behavior.
4. Dispatcher callback IDs remain structurally compatible with TaskRunner;
   operation identity remains owned by the existing completion callback.

## Alternatives considered

- **Keep the duplicate closures in MainWindow:** rejected; callback identity is
  already part of the save coordinator's lifecycle contract.
- **Create a second save-dispatch coordinator:** rejected; it would split one
  save lifecycle across parallel classes.
- **Move document snapshots or TaskRunner into the coordinator:** rejected;
  it would move concrete service/editor policy across the Qt-free boundary.
- **Generalize all document operations at once:** rejected; it would widen the
  slice beyond save callback binding and increase regression surface.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the embedded workflow and simplifier were reviewed
for applicability and no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Jason the 3rd / Luna max; bounded read-only wait timed out and the
  agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS is
  claimed.
- Independent review: Euclid the 3rd / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for typed contract, callback identity, stale and
  liveness order, read-only release, invalid/failure projection,
  continuation ordering, exception propagation, and dependency direction.
- Simplification assessment: PASS. The existing lifecycle coordinator is
  extended; no state, helper coordinator, generic runner, or compatibility
  shim is introduced.

## Verification target and limits

- Required and authorized here: source/dependency/order probe, inline valid,
  invalid, failure, stale, continuation, and exception dispatch probe,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, and expected release NO-GO evidence.
- Not proven: native TaskRunner timing, filesystem durability, QApplication
  startup, screenshots, accessibility/DPI/font rendering, clean-machine/
  cross-machine behavior, signing, installer, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
