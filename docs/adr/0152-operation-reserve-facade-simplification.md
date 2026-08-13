# ADR-0152: Operation-reserve facade simplification

- **Status:** accepted-with-limits; D117 / ARCH-91 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow._next_operation_id()` only forwarded to the existing
framework-neutral `OperationTracker.reserve()` method. The facade added no
busy, status, stale, close, or error policy, while SessionSaveCoordinator and
several application flows already depended on the same reserve contract.

## Decision

Initialize `OperationTracker` before the session-save coordinator is composed,
inject its bound `reserve` method directly into `SessionSaveCoordinator`, and
replace the seven pure facade call sites with direct tracker reservation.
Remove `_next_operation_id()`. Keep `_begin_operation()` and
`_complete_operation()` because they retain MainWindow-owned busy/status
projection and active-operation completion policy.

## Invariants

1. Every former `_next_operation_id()` call still obtains an ID from the same
   `OperationTracker` instance and retains monotonic allocation.
2. Session-save construction receives the same callable shape and still
   reserves only when its tracker admits a request.
3. `_begin_operation()` still sets busy/WORKING and `_complete_operation()`
   still clears the active operation and synchronizes status.
4. No coordinator imports Qt or receives a widget; the dependency direction
   remains `presentation -> application -> domain`.
5. No worker dispatch, stale guard, cancellation, close gate, persistence,
   notification, or document policy changes.
6. The operation tracker remains a lifecycle value owner; MainWindow remains
   the policy owner for busy/status/task behavior.

## Alternatives considered

- **Keep the forwarding facade:** rejected; it preserves indirection without
  a policy boundary.
- **Move all busy/status policy into OperationTracker:** rejected; that would
  widen a framework-neutral lifecycle value object into UI policy.
- **Create a new operation service:** rejected; the existing tracker is the
  canonical reserve/completion source and already covers the contract.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Ohm the 4th / Luna max; two bounded read-only waits timed out and
  the agent was closed without a conclusion. Status is NO_CONCLUSION; no child
  architecture PASS is claimed.
- Independent review: Einstein the 4th / Luna max; two bounded read-only
  waits timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for all reserve call sites, construction order,
  SessionSaveCoordinator callable compatibility, and preserved busy/status
  policy.
- Simplification assessment: PASS. Removing one no-policy facade reduces
  indirection while retaining the existing tracker and coordinator contracts.

## Verification target and limits

- Authorized evidence: reserve/call-site/construction-order/behavior-boundary
  probes, compileall, Ruff, format, presentation-contract audit, package
  identity, handoff/register/index synchronization, no-process evidence, and
  expected release NO-GO evidence.
- Not proven: runtime callback interleaving, QApplication startup, native UI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
