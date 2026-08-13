# ADR-0156: Workspace-navigation ports contract

- **Status:** accepted-with-limits; D120 / ARCH-94 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`WorkspaceNavigationCoordinator` already isolated completion classification,
but its constructor accepted seven callbacks positionally. That obscured the
ownership of tracker completion, surface projection, valid-result projection,
session-restore continuation, and notification policy at the MainWindow
composition boundary.

## Decision

Introduce the frozen/slotted `WorkspaceNavigationPorts` contract. The existing
coordinator now receives one ports value and uses its named callbacks for open,
directory, invalidation, stale, failure, surface, and notification paths.
MainWindow remains the composition root and maps the existing projection and
policy callbacks explicitly.

## Invariants

1. `submit_open` and `submit_directory` bind the same generation and worker
   callbacks; `complete_open`, `complete_directory`, and `fail` retain their
   existing complete/finish/stale/invalidated ordering.
2. Workspace loading, invalid-result messages, failure messages,
   session-restore continuation, valid open/directory projection, and close
   policy remain unchanged.
3. `WorkspaceNavigationPorts` contains callbacks and the existing tracker
   reference only; it owns no new workflow state or asynchronous behavior.
4. The coordinator remains Qt-free and imports no widget, service, singleton,
   or test-only asset.

## Alternatives considered

- **Keep positional callbacks:** rejected; the contract remains easy to wire
  incorrectly and hides policy ownership at composition time.
- **Move workspace policy into the coordinator:** rejected; service,
  admission, filesystem, and close policy remain MainWindow/application
  responsibilities.
- **Create a second navigation coordinator:** rejected; the existing
  completion boundary already owns the correct behavior.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Copernicus the 4th / Luna max; two bounded read-only waits timed
  out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no child architecture PASS is claimed.
- Independent review: Zeno the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for named mapping, callback order, stale/invalidated
  behavior, Qt-free boundary, and unchanged MainWindow policy ownership.
- Simplification assessment: PASS. One explicit ports contract removes
  positional coupling without adding a state or worker layer.

## Verification target and limits

- Authorized evidence: Qt-free navigation behavior and contract probes,
  compileall, Ruff, format, presentation audit, package identity,
  handoff/register/index synchronization, no-process evidence, and expected
  release NO-GO evidence.
- Not proven: QApplication startup, native workspace rendering, real worker
  interleaving, filesystem timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
