# ADR-0155: Workspace file activation boundary

- **Status:** accepted-with-limits; D119 / ARCH-93 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`WorkspacePanel` already emits distinct file, directory, and keyboard
activation intents, but `MainWindow._open_workspace_file` still combined raw
input validation, startup/busy admission, workspace containment, duplicate-tab
focus, notifications, and asynchronous document opening in one shell method.
That made the file-opening bug surface and its policy boundary harder to trace.

## Decision

Add the Qt-free generic `WorkspaceFileActivationCoordinator[TabT]` with a
frozen/slotted `WorkspaceFileActivationPorts[TabT]` contract. The coordinator
preserves the existing decision order and routes an accepted path through the
existing `MainWindow._start_open` callback. `WorkspaceSurface.file_requested`
now binds directly to the coordinator; the former shell method is removed.

## Invariants

1. Non-`Path` inputs are ignored; startup restore warns and stops; busy state
   remains silent; containment failure warns; an already-open path focuses the
   existing tab then reports it; only a new accepted path reaches `_start_open`.
2. `WorkspacePanel` retains first-click file activation, double-click
   directory/file activation, Enter/Return activation, item-kind filtering,
   and signal identities.
3. `WorkspaceService` remains the application owner of workspace root
   containment; `DocumentTabSurface` remains the tab identity owner;
   `MainWindow` remains the Qt/service composition and async-open owner.
4. The new module imports no Qt, owns no widget/service/state singleton, and
   does not create another worker or file-opening path.
5. Notification text, levels, duplicate focus ordering, startup/busy guards,
   and `_start_open` behavior remain unchanged.

## Alternatives considered

- **Keep all policy in MainWindow:** rejected; the file intent boundary remains
  mixed with the shell's unrelated composition and async lifecycle.
- **Move file opening into WorkspaceService:** rejected; service code must not
  own tabs, notifications, startup gates, or Qt presentation policy.
- **Create a second document-open worker path:** rejected; the existing
  DocumentOpenCoordinator and `_start_open` boundary are already canonical.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Newton the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`;
  no child architecture PASS is claimed.
- Independent review: Mendel the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for activation order, signal semantics, construction
  wiring, Qt-free dependency direction, and preserved `_start_open` boundary.
- Simplification assessment: PASS. The extraction removes one mixed shell
  policy method without duplicating workspace or document state.

## Verification target and limits

- Authorized evidence: six-path Qt-free activation behavior probe, source and
  signal contract probes, compileall, Ruff, format, presentation audit,
  package identity, handoff/register/index synchronization, no-process
  evidence, and expected release NO-GO evidence.
- Not proven: QApplication startup, native tree interaction, real worker
  interleaving, filesystem timing, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
