# ADR-0159: Close-guard ports contract

- **Status:** accepted-with-limits; D121 / ARCH-97 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`CloseGuardCoordinator` already owned the ordered, Qt-free close-readiness
classification, but its constructor accepted eight positionally ordered
callbacks. That made the MainWindow composition boundary harder to audit and
easier to miswire as new background operations are added.

## Decision

Introduce the frozen/slotted `CloseGuardPorts` contract. The coordinator now
receives named busy, workspace-search, dirty-tab, background, session-save,
pending-work, and timer callbacks. It retains the existing close decision and
all side-effect ordering; MainWindow remains the composition, QCloseEvent, and
application-policy owner.

## Invariants

1. `evaluate()` preserves the exact precedence: operation, workspace-search
   cancellation, dirty tabs, background work, immediate session save/pending
   work, then timer stop and allow.
2. `CloseGuardDecision` and `CloseBlockReason` remain unchanged.
3. The ports value owns no state or behavior beyond references to existing
   callbacks; the coordinator remains Qt-free.
4. MainWindow close event acceptance/ignore behavior, messages, services,
   trackers, timers, and persistence policy remain unchanged.

## Alternatives considered

- **Keep positional callbacks:** rejected; field order remains a wiring risk.
- **Move close policy into the ports object:** rejected; it would create a
  second policy/state owner and weaken the current boundary.
- **Add a generic dependency-injection framework:** rejected; the explicit
  frozen contract is sufficient and keeps the file architecture small.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Avicenna the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Gibbs the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named MainWindow mapping, branch/order preservation,
  Qt-free dependency direction, and unchanged close-event policy.
- Simplification assessment: PASS. One explicit ports value removes positional
  coupling without introducing a second state or policy layer.

## Verification target and limits

- Authorized evidence: all close-guard branch/order and source-contract probes,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native QCloseEvent timing, QApplication startup, actual worker
  interleaving, filesystem timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
