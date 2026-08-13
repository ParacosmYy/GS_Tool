# ADR-0160: Document-open ports contract

- **Status:** accepted-with-limits; D122 / ARCH-98 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentOpenCoordinator` already classified ordinary and session-restore
completion callbacks, but its constructor accepted seven callbacks positionally.
The ordering mixed operation completion, restore identity, projection,
continuation, error, and notification concerns at the MainWindow composition
site.

## Decision

Introduce the frozen/slotted `DocumentOpenPorts` contract. The coordinator now
receives named callbacks for completion, restore identity/document consumption,
valid projection, restore continuation, errors, and notification. MainWindow
remains the composition and application-policy owner; projection stays in the
existing `DocumentOpenProjectionCoordinator`.

## Invariants

1. Ordinary valid/invalid/failure and session-restore valid/invalid/failure
   branches preserve operation completion, session-document take, notification,
   error, apply, and continuation order.
2. `submit()` retains its dispatcher and line-number contract.
3. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.
4. DocumentService, TaskRunner, tab/editor projection, persistence, startup,
   close, and application policy remain at their existing owners.

## Alternatives considered

- **Keep positional callbacks:** rejected; the mixed contract remains easy to
  miswire as restore behavior evolves.
- **Move projection into the ports object:** rejected; it would duplicate or
  blur the existing projection coordinator boundary.
- **Create a second document-open workflow:** rejected; the existing
  coordinator already owns the correct classification boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Rawls the 4th / Luna max; two bounded read-only waits timed out and
  the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Goodall the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, ordinary/restore branch order,
  projection ownership, Qt-free boundary, and unchanged application policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating state or projection behavior.

## Verification target and limits

- Authorized evidence: ordinary/restore open behavior and source-contract
  probes, compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native editor rendering, QApplication startup, real worker
  interleaving, filesystem decoding timing, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
