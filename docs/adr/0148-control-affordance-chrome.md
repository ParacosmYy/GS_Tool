# ADR-0148: Control-affordance chrome

- **Status:** accepted-with-limits; UI-59 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The shared QSS gives `QComboBox` and `QSpinBox` readable value surfaces and
focus borders, but their native dropdown and spinner subcontrols retain a
low-signal platform appearance. This weakens the visual hierarchy of the
settings controls and makes value-changing affordances harder to scan.

## Decision

Style the existing `QComboBox::drop-down` and
`QAbstractSpinBox::up-button/down-button` subcontrols in the centralized
stylesheet. Use canonical surface/border/accent/pressed tokens for base,
hover, pressed, and disabled states; explicitly position the spinner buttons
and retain the native arrow glyphs and control semantics.

## Invariants

1. No settings value range, signal, editor/application policy, locale,
   persistence, keyboard focus, or accessibility contract changes.
2. QSS remains centralized in `theme.py`; no widget code or new visual state
   owner is introduced.
3. Existing input, focus, read-only, disabled, popup, and field identity
   selectors remain in place; the new rules only add subcontrol chrome.
4. Every new color is a `ThemeColors` token; no hard-coded endpoint is added.
5. Native Qt subcontrol painting, arrow glyph visibility, DPI, fonts,
   accessibility, and actual visual perception remain runtime limits.

## Alternatives considered

- **Leave native subcontrols untouched:** rejected; it preserves the reported
  lack of visual distinction for value-changing affordances.
- **Replace arrows with authored image assets:** rejected for this slice; it
  expands icon ownership and high-DPI validation beyond the chrome problem.
- **Add custom widgets or event handlers:** rejected; subcontrol QSS is enough
  and avoids behavior coupling.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Lorentz the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Banach the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for subcontrol scope, token-only states, preserved native
  semantics, and unchanged behavior boundaries.
- Simplification assessment: PASS. Keeping the subcontrol states in the
  existing stylesheet avoids custom widgets, event handlers, and assets.

## Verification target and limits

- Authorized evidence: subcontrol source probe, token-state probe,
  compileall, Ruff, format, presentation-contract audit, package identity,
  handoff/register/index synchronization, no-process evidence, and expected
  release NO-GO evidence.
- Not proven: native subcontrol rendering, arrow visibility, actual geometry,
  keyboard/screen-reader output, DPI/font behavior, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
