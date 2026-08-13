# ADR-0146: Dialog-shell edge hierarchy

- **Status:** accepted-with-limits; UI-58 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Settings and Command Palette dialogs already have stable semantic object
names and centralized child-control styling. Their outer client surfaces did
not have a visible frame/top accent, so the visual hierarchy depended on the
platform dialog surface and appeared less intentional than the existing
plugin and workspace-search surfaces.

## Decision

Add only two object-scoped QSS contracts to `presentation.theme`: a shared
surface/border/radius frame for `QDialog#settingsDialog` and
`QDialog#commandPalette`, plus distinct token-driven top accents. Settings
uses `accent_alt`; Command Palette uses `accent_pink`. Existing child
selectors remain unchanged and continue to own fields, lists, buttons,
preview content, and action roles.

## Invariants

1. No dialog constructor, signal, command selection, settings snapshot,
   persistence, locale, focus order, layout, or application policy changes.
2. QSS remains centralized in `theme.py`; no dialog imports theme code and no
   new visual state owner is introduced.
3. The selectors are object-scoped and do not change generic `QDialog` or
   native plugin/workspace-search contracts.
4. All frame and accent colors come from canonical `ThemeColors` tokens; no
   fixed endpoint color is added.
5. Native Qt frame rendering, DPI, fonts, accessibility, and actual visual
   perception remain acceptance limits until authorized runtime evidence exists.

## Alternatives considered

- **Leave the platform dialog surface:** rejected; it preserves the reported
  lack of a clear outer hierarchy.
- **Add per-dialog widgets or a shared base dialog:** rejected; the existing
  object names and centralized QSS are sufficient and a widget abstraction
  would expand ownership without behavior value.
- **Restyle every child control in this slice:** rejected; child selectors
  already have semantic contracts and widening the diff would raise cascade
  risk.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Hypatia the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Anscombe the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for selector scope, cascade order, token-only colors,
  preserved child ownership, and unchanged behavior boundaries.
- Simplification assessment: PASS. Keeping the two-selector frame contract in
  the existing stylesheet is smaller and clearer than a new dialog base class,
  wrapper widget, or state owner.

## Verification target and limits

- Authorized evidence: selector/source probe, token contrast/readability
  probe, compileall, Ruff, format, presentation-contract audit, package
  identity, handoff/register/index synchronization, no-process evidence, and
  expected release NO-GO evidence.
- Not proven: QApplication startup, native QSS rendering, actual dialog
  geometry, accessibility, DPI/font behavior, clean-machine/cross-machine
  behavior, signing, installer, updater, legal, support, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
