# ADR-0149: Dialog action-rail hierarchy

- **Status:** accepted-with-limits; UI-60 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

Plugin Catalog and Plugin Status already assign primary and warning roles to
their mutation buttons, but the two action rows were bare `QHBoxLayout`
instances. Without a presentation identity or separator, the action boundary
was visually weak and inconsistent with the settings/search dialog action
rail.

## Decision

Wrap each existing action layout in a `QWidget#dialogActionRail`, preserve the
same button instances, connections, enablement, and layout order, and add one
centralized token-driven top border plus minimum button width. The wrapper is
presentation-only and does not own plugin policy or button semantics.

## Invariants

1. Approve/revoke/enable/disable signal connections, selected-row logic,
   governance enablement, locale text, plugin data, and policy are unchanged.
2. The wrapper has no new signals, state, events, or application dependencies.
3. `theme.py` remains the sole visual-token/QSS owner; object-scoped rules do
   not broaden generic button behavior.
4. Existing primary/warning button selectors remain responsible for action
   state and contrast.
5. Native Qt layout metrics, rendering, DPI, fonts, accessibility, and visual
   perception remain runtime limits.

## Alternatives considered

- **Leave bare layouts:** rejected; the action boundary stays visually weak.
- **Add a reusable action-bar component:** rejected; two existing rows need one
  presentation identity and a component would add ownership without behavior.
- **Move button policy into the wrapper:** rejected; it would violate the
  existing plugin surface/policy boundary.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Peirce the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Ramanujan the 4th / Luna max; two bounded read-only
  waits timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for wrapper scope, preserved button/signal ownership,
  token-only QSS, and unchanged behavior boundaries.
- Simplification assessment: PASS. One shared object-name/QSS contract is
  smaller than a new component or per-dialog policy logic.

## Verification target and limits

- Authorized evidence: action-rail source/signal probe, token separator
  probe, compileall, Ruff, format, presentation-contract audit, package
  identity, handoff/register/index synchronization, no-process evidence, and
  expected release NO-GO evidence.
- Not proven: native layout/rendering, accessibility/focus output, DPI/font
  behavior, clean-machine, cross-machine, signing, installer, updater, legal,
  support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
