# ADR-0153: Session-restore ports contract

- **Status:** accepted-with-limits; D118 / ARCH-92 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`SessionRestoreCoordinator[TabT]` already owned ordered, Qt-free restore
progression, but its constructor accepted ten presentation callbacks as
positionally ordered parameters. That made `MainWindow` composition harder to
review and allowed a callback-order mistake to look type-correct at the call
site.

## Decision

Introduce the frozen, slotted generic `SessionRestorePorts[TabT]` contract.
The coordinator now receives the tracker and one ports value, and reads all
restore callbacks from that value. `MainWindow` remains the composition root
and maps each callback by an explicit field name.

## Invariants

1. The coordinator remains Qt-free and owns no widget, service, persistence,
   notification, or startup policy.
2. Inactive and workspace-pending guards remain first; ordered deferred,
   duplicate, pending-open, active-tab, initial-document, finish, and save
   sequencing is unchanged.
3. The tracker remains the only restore-state owner; `SessionRestorePorts`
   contains callbacks only and has no mutable workflow state.
4. `MainWindow` retains session/document/workspace services, TaskRunner/open
   callbacks, tab projection, startup/close state, and notification policy.
5. No compatibility adapter, service locator, global singleton, or new
   asynchronous path is introduced.

## Alternatives considered

- **Keep positional callbacks:** rejected; the contract remains easy to wire
  incorrectly and obscures ownership at the composition boundary.
- **Move restore policy into the ports object:** rejected; that would turn a
  callback contract into a second state owner.
- **Create another restore service/coordinator:** rejected; the existing
  Qt-free coordinator already owns the correct behavior boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: McClintock the 4th / Luna max; two bounded read-only waits timed
  out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no child architecture PASS is claimed.
- Independent review: Kierkegaard the 4th / Luna max; two bounded read-only
  waits timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for the single call site, named callback mapping,
  Qt-free dependency boundary, and preserved restore order.
- Simplification assessment: PASS. One typed ports value reduces positional
  constructor coupling without adding a second policy or state owner.

## Verification target and limits

- Authorized evidence: AST/source contract probes, Qt-free restore behavior
  probe, compileall, Ruff, format, presentation audit, package identity,
  handoff/register/index synchronization, no-process evidence, and expected
  release NO-GO evidence.
- Not proven: QApplication startup, native rendering, callback timing under
  real workers, clean-machine, cross-machine, signing, installer, updater,
  legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
