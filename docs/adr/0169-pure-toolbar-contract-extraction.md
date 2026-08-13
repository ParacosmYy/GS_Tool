# ADR-0169: Pure toolbar contract extraction

- **Status:** accepted-with-limits; D130 / ARCH-107 bounded architecture correction
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

D129 moved the core toolbar action-spec composition out of `MainWindow`, but
the new coordinator imported `ToolbarActionSpec` from `command_surface.py`
and `IconKey` from `icons.py`. Both modules are Qt-bearing projection/render
modules, so the coordinator had no widgets but still retained a transitive
PyQt6 dependency.

## Decision

Introduce two small pure-Python contract modules:

- `icon_contract.py` owns `IconKey`.
- `toolbar_contract.py` owns `ToolbarActionRole` and `ToolbarActionSpec`.

`icons.py` re-exports `IconKey`, and `command_surface.py` imports and exposes
the toolbar contract names at their existing seams for compatibility. The
`CoreToolbarCoordinator` imports only the pure contract modules. Qt rendering,
translation, QAction/QToolBar creation, and state projection remain in the
existing presentation surfaces.

## Invariants

1. `CoreToolbarCoordinator` and the contract modules import without loading
   PyQt6.
2. Existing imports from `presentation.icons` and `presentation.command_surface`
   continue to resolve the same contract names.
3. No-workspace and workspace toolbar action counts, order, text keys,
   callbacks, icons, separators, and roles remain unchanged.
4. `CommandSurface` remains the sole owner of QToolBar/QAction construction,
   locale projection, icon rendering, and Qt state.
5. No command registry, plugin policy, application state, shortcut behavior,
   or global singleton is introduced.

## Alternatives considered

- **Leave the transitive Qt import:** rejected; it undermines the declared
  coordinator boundary and makes pure contract inspection depend on Qt.
- **Move all icon rendering into the coordinator:** rejected; it would invert
  the projection boundary and couple composition to pixels/widgets.
- **Create a generic presentation framework:** rejected; two focused contract
  modules are sufficient and preserve the existing public seams.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation contract work. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Leibniz the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. The follow-up compatibility
  export assessment by Lovelace the 4th / Luna max also timed out and was
  closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Poincare the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. The final
  compatibility-export review by Godel the 4th / Luna max also timed out and
  was closed without a conclusion. Status is `NO_CONCLUSION`; no independent
  PASS is claimed.
- Parent review: PASS for dependency direction, compatibility imports, exact
  toolbar metadata/order, and unchanged Qt projection ownership.
- Simplification assessment: PASS. Two focused contracts remove only the
  transitive Qt coupling; no generic abstraction or duplicated policy was
  introduced.

## Verification target and limits

- Authorized evidence: pure-contract import probe, toolbar behavior probe,
  compileall, Ruff, format, project checks, package identity, root/dist
  identity, no-process evidence, handoff/register/index synchronization, and
  expected release NO-GO evidence.
- Not proven: native QToolBar/QAction rendering, runtime shortcut delivery,
  locale retranslation timing, DPI/font metrics, accessibility, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
