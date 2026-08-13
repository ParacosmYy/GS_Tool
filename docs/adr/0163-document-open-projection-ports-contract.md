# ADR-0163: Document-open projection ports contract

- **Status:** accepted-with-limits; D125 / ARCH-101 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentOpenProjectionCoordinator` already owned valid-open tab, duplicate,
line, cursor, event, notification, and session-restore projection, but its
constructor accepted nine presentation callbacks positionally. The ordering
was difficult to read at the MainWindow composition site.

## Decision

Introduce the frozen/slotted generic `DocumentOpenProjectionPorts[TabT]`
contract. The coordinator now receives named callbacks for existing-tab lookup,
restore recording, duplicate error, tab creation, line navigation, cursor
placement, event publication, notification, and restore continuation. MainWindow
remains the composition and application-policy owner.

## Invariants

1. Existing restored tabs still record and continue restore without creating a
   duplicate tab; ordinary duplicates still show the existing error and stop.
2. New tabs still receive optional line navigation first, then optional restore
   cursor/recording, followed by event, notification, and restore continuation.
3. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.
4. `TabT`, `Path | None`, `OpenedDocument`, and `SessionDocument` contracts
   remain explicit; DocumentService, open classification, persistence, startup,
   close, and application policy remain at existing owners.

## Alternatives considered

- **Keep positional callbacks:** rejected; the branch and projection contract
  remains easy to miswire as restore behavior evolves.
- **Move tab/editor policy into the ports object:** rejected; it would blur the
  existing projection boundary and make the ports a second policy owner.
- **Introduce a second open projection workflow:** rejected; the existing
  coordinator already owns the correct valid-result boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Hubble the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Parfit the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, duplicate/restored branch order,
  line/cursor projection, Qt-free boundary, and unchanged application policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating tab or restore state.

## Verification target and limits

- Authorized evidence: duplicate/restored/new open projection behavior and
  source-contract probes, compileall, Ruff, format, package identity,
  handoff/register/index synchronization, no-process evidence, and expected
  release NO-GO evidence.
- Not proven: native editor rendering, QApplication startup, real worker
  interleaving, filesystem decoding timing, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
