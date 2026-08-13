# ADR-0161: Document-save ports contract

- **Status:** accepted-with-limits; D123 / ARCH-99 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentSaveCoordinator` already classified asynchronous save callbacks, but
its constructor accepted five callbacks positionally. The ordering mixed
operation completion, tab liveness, editor lock release, valid-save projection,
and error projection at the MainWindow composition site.

## Decision

Introduce the frozen/slotted generic `DocumentSavePorts[TabT]` contract. The
coordinator now receives named callbacks for operation completion, live-tab
containment, read-only release, valid-save projection, and error projection.
MainWindow remains the composition and application-policy owner; the existing
`DocumentSaveProjectionCoordinator` remains the valid-result projection owner.

## Invariants

1. Stale operation IDs and missing tabs still short-circuit before any tab or
   error side effect.
2. A live save callback still releases read-only state before validating the
   returned `DocumentState`; invalid results still show the existing error.
3. Valid results still reach the existing projection with the same continuation
   object, and failures still preserve the existing error text.
4. `submit()` retains its dispatcher, operation ID, tab, continuation, and
   callback-binding contract.
5. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.

## Alternatives considered

- **Keep positional callbacks:** rejected; the mixed contract remains easy to
  miswire as save projection evolves.
- **Move save projection into the ports object:** rejected; it would duplicate
  or blur the existing `DocumentSaveProjectionCoordinator` boundary.
- **Introduce a second save workflow:** rejected; the existing coordinator
  already owns the correct classification boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Kepler the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Volta the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, save branch order, projection
  ownership, Qt-free boundary, and unchanged application policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating state or projection behavior.

## Verification target and limits

- Authorized evidence: ordinary valid/invalid/failure/stale/liveness save
  behavior and source-contract probes, compileall, Ruff, format, package
  identity, handoff/register/index synchronization, no-process evidence, and
  expected release NO-GO evidence.
- Not proven: native editor rendering, QApplication startup, real worker
  interleaving, filesystem durability/timing, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
