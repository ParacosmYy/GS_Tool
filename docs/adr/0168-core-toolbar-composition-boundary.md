# ADR-0168: Core toolbar composition boundary

- **Status:** accepted-with-limits; D129 / ARCH-106 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

After D128 moved the built-in command catalog out of `MainWindow`, the same
window still assembled the six core toolbar action specifications and the
optional workspace action. The actual `QToolBar`, `QAction`, icon tint,
translation, and Qt state projection already belonged to `CommandSurface`.

## Decision

Introduce the presentation-only `CoreToolbarCoordinator` with frozen/slotted `CoreToolbarPorts`. It
composes the existing `ToolbarActionSpec` values in their established order
and appends the workspace action only when a workspace callback is available.
`MainWindow` maps existing callbacks; `CommandSurface` remains the only Qt
toolbar projection owner.

## Invariants

1. Without a workspace, the action list contains exactly New, Open, Save, Find,
   Replace, and Command Palette in the existing order.
2. With a workspace, the context Workspace action is appended last.
3. Existing `text_key`, callback, `IconKey`, `separator_before`, and role
   values remain unchanged.
4. CommandSurface retains toolbar creation, QAction signals, locale
   retranslation, authored icon tinting, and native Qt state projection.
5. The new coordinator owns no Qt widget, command registry, plugin policy,
   application state, shortcut behavior, or global singleton. D130 separately
   extracts the shared value contracts so this boundary also has no transitive
   PyQt6 import.

## Alternatives considered

- **Keep specs in MainWindow:** rejected; it leaves presentation metadata
  coupled to the largest application orchestrator after the command catalog
  has already been separated.
- **Move action assembly into CommandSurface:** rejected; it would couple the
  Qt projection class to MainWindow behavior and workspace policy.
- **Create a generic toolbar builder:** rejected; the bounded core action list
  needs a small named contract, not a configurable framework or second command
  strategy.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation composition. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Averroes the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Mencius the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for six/seven action coverage, order, metadata, named
  callback mapping, CommandSurface ownership, and unchanged locale/icon path.
- Simplification assessment: PASS. One small coordinator removes only the
  action-spec composition cluster; no new Qt abstraction or command policy is
  introduced.

## Verification target and limits

- Authorized evidence: toolbar behavior/contract probes, compileall, Ruff,
  format, project checks, package identity, root/dist identity, no-process
  evidence, handoff/register/index synchronization, and expected release
  NO-GO evidence.
- Not proven: native QToolBar/QAction rendering, runtime shortcut delivery,
  locale retranslation timing, DPI/font metrics, accessibility, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
