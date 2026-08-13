# ADR-0138: Document-open dispatch callback boundary

- **Status:** accepted-with-limits; D111/ARCH-83 bounded architecture slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`DocumentOpenCoordinator` already owns open-result classification: operation
liveness, ordinary versus session-restore identity, invalid-result and error
projection, restore continuation, document application, and optional line
navigation. MainWindow repeated success/failure closures only to bind the
requested line number before submitting `DocumentService.open_document()`.

## Decision

Extend the existing Qt-free coordinator with typed `OpenOperation`,
`OpenSuccess`, `OpenFailure`, and `OpenDispatcher` contracts plus a
keyword-only `submit(...)` method. The method binds the optional line number
to the existing `complete()` callback and binds failure to `fail()`, then
invokes a structural dispatcher supplied by MainWindow.

MainWindow now passes the concrete open operation and `TaskRunner.submit`
while retaining path selection, operation-ID allocation, session-restore
binding, DocumentService, busy/status/notification, persistence, and close
policy. If dispatch raises synchronously, it propagates without synthesizing a
completion or changing restore state.

## Invariants

1. Existing operation-ID stale suppression, ordinary/session-restore
   classification, invalid-result/error projection, restore continuation,
   document application, and line navigation are unchanged.
2. The coordinator remains Qt-free and imports neither TaskRunner,
   DocumentService, filesystem, nor widgets.
3. MainWindow keeps path selection, session-restore binding, service/runner
   ownership, busy/status/notification, persistence, and close behavior.
4. Dispatcher callback IDs remain structurally compatible with TaskRunner;
   the coordinator only binds presentation identity needed by its existing
   lifecycle methods.

## Alternatives considered

- **Keep the duplicate closures in MainWindow:** rejected; callback identity
  and line navigation are already part of the open coordinator lifecycle.
- **Create a second open-dispatch coordinator:** rejected; it would split
  ordinary and session-restore completion across parallel owners.
- **Move DocumentService or TaskRunner into the coordinator:** rejected; it
  would move concrete service/thread policy across the Qt-free boundary.
- **Generalize all document operations at once:** rejected; it would widen the
  slice beyond open callback binding and increase regression surface.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the embedded workflow and simplifier were reviewed
for applicability and no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Hilbert the 3rd / Luna max; bounded read-only wait timed out and
  the agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS
  is claimed.
- Independent review: Newton the 3rd / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for typed contract, callback identity, stale and
  restore branching, line-number propagation, invalid/failure projection,
  continuation ordering, exception propagation, and dependency direction.
- Simplification assessment: PASS. The existing open lifecycle coordinator is
  extended; no state owner, helper coordinator, generic runner, or
  compatibility shim is introduced. Explicit success/failure closures are
  retained because they make the line-number binding and failure mapping
  visible at the contract boundary.

## Verification target and limits

- Required and authorized here: source/dependency/order probe, inline
  ordinary/session-restore valid, invalid, failure, stale, line-navigation,
  continuation, and exception dispatch probe, compileall, Ruff, format,
  package identity, handoff/register/index synchronization, and expected
  release NO-GO evidence.
- Not proven: native TaskRunner timing, filesystem/document decoding,
  QApplication startup, screenshots, accessibility/DPI/font rendering,
  clean-machine/cross-machine behavior, signing, installer, legal, support,
  or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
