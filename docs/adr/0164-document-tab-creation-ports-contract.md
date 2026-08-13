# ADR-0164: Document-tab creation ports contract

- **Status:** accepted-with-limits; D126 / ARCH-102 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentTabCreationCoordinator` already owned deterministic tab assembly, but
its constructor accepted eight presentation callbacks positionally. The
composition site mixed editor creation, concrete tab construction, surface
insertion, metadata queries, title refresh, persistence, and status projection.

## Decision

Introduce the frozen/slotted generic
`DocumentTabCreationPorts[OpenedT, TabT, EditorT]` contract. The coordinator now
receives named callbacks for each existing assembly seam. MainWindow remains the
composition and application-policy owner; concrete editor, tab, persistence,
and status adapters remain outside the Qt-free coordinator.

## Invariants

1. `add()` preserves the exact order: create editor, create tab with the
   optional recovery snapshot ID, query title and modified state while adding
   the tab, update title, request session save, synchronize status, return tab.
2. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.
3. `OpenedT`, `TabT`, and `EditorT` generic contracts remain explicit, and
   recovery snapshot identity is forwarded unchanged.
4. EditorDocumentSurface, DocumentTabSurface, recovery, persistence, startup,
   close, and application policy remain at their existing owners.

## Alternatives considered

- **Keep positional callbacks:** rejected; the assembly contract remains easy
  to miswire as tab metadata evolves.
- **Move concrete editor/tab creation into the coordinator:** rejected; it
  would introduce Qt/editor policy and reverse the presentation boundary.
- **Introduce a factory hierarchy:** rejected; one typed callback object is the
  smallest complete contract for the existing single assembly workflow.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Raman the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Dalton the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, exact assembly order, recovery ID
  forwarding, Qt-free boundary, and unchanged application policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating concrete tab/editor state.

## Verification target and limits

- Authorized evidence: normal/recovery tab-assembly behavior and source-contract
  probes, compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native editor rendering, QApplication startup, real worker
  interleaving, filesystem/recovery durability, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
