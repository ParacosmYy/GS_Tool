# ADR-0144: Workspace-dock chrome hierarchy

- **Status:** accepted-with-limits; UI-57 / ARCH-88 bounded visual slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace dock already had a stable WorkspaceDock object name, but the
native dock frame and title-bar buttons were left to generic styling. That
made the workspace surface visually inconsistent with the token-driven panel,
toolbar, tabs, and status rail.

## Decision

Scope the existing dock styling to QDockWidget#WorkspaceDock and add explicit
token-driven rules for the dock frame, title, close button, and float button.
Hover, pressed, and disabled states use existing surface, border, accent, and
pressed tokens. WorkspaceSurface retains the existing object name and all
docking/panel composition.

## Invariants

1. No new widget, dock policy, close policy, signal, panel state, or navigation
   behavior is introduced.
2. The styling is scoped to WorkspaceDock; no global QDockWidget contract is
   added.
3. Native close/float icons remain native; QSS only controls their authored
   surface, border, geometry, and interaction states.
4. WorkspaceSurface retains locale projection, panel callbacks, dock placement,
   and MainWindow ownership.

## Alternatives considered

- **Leave native dock chrome untouched:** rejected; it is the remaining
  unthemed shell surface identified by the static object-name audit.
- **Style every QDockWidget globally:** rejected; scope would leak into future
  docks and weaken ownership.
- **Replace native buttons with custom widgets/icons:** rejected; it would
  change native docking affordances and add unnecessary lifecycle/state.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance workflow and simplifier are N/A for source scope; no embedded source
was changed. Public CloudWeGo material remains an engineering reference only.
No private ByteDance standard, certification, MISRA, ISO 26262, ASIL, ASPICE,
or compliance claim is made.

## Review and simplification

- Architect: Lagrange the 4th / Luna max; bounded read-only consultation timed
  out and was closed. Status is NO_CONCLUSION; no child architecture PASS is
  claimed.
- Independent review: Plato the 4th / Luna max; bounded read-only review
  timed out and was closed. Status is NO_CONCLUSION; no independent PASS is
  claimed.
- Parent source review: PASS for object-name scope, native subcontrol
  containment, token usage, interaction states, and preservation of docking,
  locale, callbacks, and close behavior.
- Simplification assessment: PASS. Scoping the existing rules and adding the
  three native subcontrol state groups is the smallest complete change; no
  custom title bar or new surface is needed.

## Verification target and limits

- Authorized evidence includes the UI-57 dock selector source probe, token
  contrast probe, compileall, Ruff, format, presentation-contract audit,
  package identity, handoff/register/index synchronization, repository checks,
  no-process evidence, and expected release NO-GO evidence.
- Not proven: native QDockWidget subcontrol painting/positioning, actual
  docking/floating interaction, screen-reader output, DPI/font metrics,
  QApplication startup, clean-machine/cross-machine behavior, signing,
  installer, updater, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
