# ADR-0157: Shell elevation and visual rhythm

- **Status:** accepted-with-limits; UI-64 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The centralized stylesheet already supplied semantic states, but the command
rail and document-tab rail read as flat strips. Their primary and selected
states were readable, yet the shell lacked a clear floating surface, consistent
touch/keyboard target rhythm, and a visually grouped document navigator.

## Decision

Keep the change inside `presentation/theme.py` and reuse the existing surface,
border, accent, and text tokens. Give `QToolBar#commandBar` a rounded elevated
rail with slightly larger authored button targets, and give
`QTabBar#documentTabBar` a rounded container with an inset tab rhythm. Existing
hover, focus, pressed, selected, and disabled selectors remain the state
owners.

## Invariants

1. This is QSS-only; no widget construction, object name, signal, shortcut,
   layout owner, locale, settings schema, or application policy changes.
2. Every visual value is derived from existing `ThemeColors` tokens or the
   existing font-size parameter; no second palette or theme branch is added.
3. Command roles, tab identity/current-index behavior, close affordance,
   keyboard focus, disabled states, and document lifecycle remain unchanged.
4. The stylesheet remains valid for all Ink/Violet, Paper/Sand, Sakura/Pop and
   Violet/Cyan/Rose/Amber combinations under the static token probe.

## Alternatives considered

- **Add a new widget shell or design-system package:** rejected; the existing
  centralized stylesheet is the correct visual seam for this bounded change.
- **Add per-theme or per-accent selector branches:** rejected; token reuse
  keeps the visual hierarchy extensible and avoids drift.
- **Use gradients or runtime effects:** rejected; native QSS effects would add
  platform/runtime variance without improving the required state contract.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation styling. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The mandatory
embedded assurance workflow and simplifier are N/A for this source scope; no
embedded source was changed. Public CloudWeGo material remains an engineering
reference only. No private ByteDance standard, certification, MISRA, ISO
26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Jason the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Hooke the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for selector scope, state preservation, token reuse,
  cross-theme source coverage, and unchanged behavior ownership.
- Simplification assessment: PASS. The smallest safe seam is the existing
  centralized stylesheet; no new visual component or token layer is needed.

## Verification target and limits

- Authorized evidence: all-theme QSS/source probe, token contrast probe,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native Qt style-engine rendering, DPI/font metrics, QApplication
  startup, accessibility tree capture, real interaction timing, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
