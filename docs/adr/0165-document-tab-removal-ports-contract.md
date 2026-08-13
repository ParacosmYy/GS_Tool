# ADR-0165: Document-tab removal ports contract

- **Status:** accepted-with-limits; D127 / ARCH-103 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`DocumentTabRemovalCoordinator` already finalized a tab after close policy
approved removal, but its constructor accepted eleven presentation callbacks
positionally. The composition site mixed liveness, recovery capture cleanup,
tab/editor lifecycle, close-event publication, persistence, and empty-state
fallback.

## Decision

Introduce the frozen/slotted generic `DocumentTabRemovalPorts[TabT, CaptureT]`
contract. The coordinator now receives named callbacks for each existing
finalization seam. MainWindow remains the composition and close-policy owner;
recovery and concrete tab/editor adapters remain outside the Qt-free
coordinator.

## Invariants

1. Missing tabs return `False` before any other callback or side effect.
2. Live tabs derive document identity, look up capture, cancel only a present
   capture, clear the snapshot, remove the tab, delete the editor, publish the
   close event, request session save, and ensure an initial document only when
   the final count is zero.
3. The coordinator ignores the existing `remove_tab` return value and returns
   `True` for an eligible live tab, preserving the established contract.
4. The ports object owns no workflow state or asynchronous behavior; the
   coordinator remains Qt-free and does not import a widget or service.
5. Close admission, recovery capture policy, persistence, startup, and
   application policy remain at existing owners.

## Alternatives considered

- **Keep positional callbacks:** rejected; the finalization contract remains
  easy to miswire as recovery and close behavior evolve.
- **Move close admission into the coordinator:** rejected; close policy is
  intentionally owned by MainWindow and its close guard.
- **Introduce a recovery-specific removal workflow:** rejected; the existing
  coordinator already owns the correct post-admission boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Dewey the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Franklin the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, missing/live/capture/empty-tab order,
  bool return semantics, Qt-free boundary, and unchanged close policy.
- Simplification assessment: PASS. One typed ports contract removes positional
  coupling without duplicating recovery or tab state.

## Verification target and limits

- Authorized evidence: missing/live/capture/empty-tab removal behavior and
  source-contract probes, compileall, Ruff, format, package identity,
  handoff/register/index synchronization, no-process evidence, and expected
  release NO-GO evidence.
- Not proven: native tab/editor rendering, QApplication startup, real worker
  interleaving, recovery/filesystem durability, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
