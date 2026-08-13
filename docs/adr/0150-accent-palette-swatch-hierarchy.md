# ADR-0150: Accent palette swatch hierarchy

- **Status:** accepted-with-limits; UI-61 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Settings dialog already offered language, theme, accent, font, size, and
motion choices, but theme and accent rows were text-only. That made palette
choices harder to scan and did not give the requested theme-color selection a
clear visual cue. The application already has a centralized `theme_colors`
resolver and an authored vector icon module.

## Decision

Add a presentation-only `color_swatch_icon(...)` renderer to
`presentation/icons.py`. SettingsDialog uses the resolved public theme tokens
to assign compact swatches to the existing Theme and Accent QComboBox items;
swatches are refreshed when the pending theme or accent changes. Existing
localized item text remains the semantic label and the domain values remain
`ThemeId`/`AccentId`.

## Invariants

1. `settings_snapshot()` returns the same schema, domain values, font values,
   editor options, and motion setting as before.
2. Locale refresh changes item text only; the swatch renderer has no locale,
   persistence, service, or application-policy dependency.
3. Theme and accent previews continue to use `theme_colors`; no duplicate
   palette table is introduced in SettingsDialog.
4. The swatch is decorative. Existing option text remains the accessibility
   and keyboard-selection contract.
5. `icons.py` remains the owner of authored vector rendering; SettingsDialog
   remains the owner of control composition and pending-value projection.
6. Native QComboBox layout, DPI, font metrics, screen-reader rendering, and
   human visual perception remain runtime limits.

## Alternatives considered

- **Keep text-only options:** rejected; the user cannot distinguish palettes
  at a glance.
- **Add color policy to the domain model:** rejected; it would duplicate the
  centralized theme resolver and widen the domain boundary.
- **Use a custom item delegate:** rejected for this bounded slice; it adds
  paint/event ownership when the existing QComboBox icon contract is enough.
- **Add image assets:** rejected; the existing vector renderer keeps the
  choices theme-aware and avoids a new resource lifecycle.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Turing the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Nietzsche the 4th / Luna max; two bounded read-only
  waits timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for the presentation-only dependency direction,
  initialization order, source compatibility, and unchanged settings
  behavior boundary.
- Simplification assessment: PASS. One generic vector swatch renderer and
  one SettingsDialog refresh method are smaller than a new widget/delegate or
  a duplicate palette model.

## Verification target and limits

- Authorized evidence: source swatch/data/boundary probes, compileall, Ruff,
  format, presentation-contract audit, package identity, handoff/register/
  index synchronization, no-process evidence, and expected release NO-GO
  evidence.
- Not proven: QApplication startup, native QComboBox rendering, accessibility
  output, DPI/font behavior, screenshots, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
