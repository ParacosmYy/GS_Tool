# ADR-0162: Document-save projection ports contract

- **Status:** accepted-with-limits; D124 / ARCH-100 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentSaveProjectionCoordinator` already owns the ordered projection of a
validated save result, but its constructor accepted seven presentation
callbacks positionally. The ordering mixed state, language, title, recovery,
event, notification, and persistence seams at the MainWindow composition site.

## Decision

Introduce the frozen/slotted generic `DocumentSaveProjectionPorts[TabT]`
contract. The coordinator now receives named callbacks for each existing
projection seam. MainWindow remains the composition and application-policy
owner; `DocumentSaveCoordinator` remains the validation/classification owner.

## Invariants

1. `project()` preserves the exact observable order: apply state, refresh
   language, update title, clear recovery, publish the saved event, notify the
   path, request session save, then invoke the optional continuation.
2. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.
3. `DocumentSaveCoordinator`, DocumentService, TaskRunner, editor/tab policy,
   persistence, startup, close, and application policy remain at their current
   owners.
4. The generic `TabT` and `Path | None` callback contracts remain explicit.

## Alternatives considered

- **Keep positional callbacks:** rejected; the ordered projection contract
  remains easy to miswire as save feedback evolves.
- **Move projection into DocumentSaveCoordinator:** rejected; classification
  and valid-result projection are already separate bounded responsibilities.
- **Introduce a second projection workflow:** rejected; the existing
  coordinator already owns the correct valid-save boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Faraday the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Halley the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, exact projection order, generic
  typing, Qt-free boundary, and unchanged application policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating projection state or adding an adapter layer.

## Verification target and limits

- Authorized evidence: ordered projection behavior and source-contract probes,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native editor rendering, QApplication startup, real worker
  interleaving, filesystem durability/timing, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
