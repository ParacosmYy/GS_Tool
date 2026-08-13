# ADR-0216: Complete Settings typography preview

- **Status:** accepted-with-limits; D167 / UI-79 / ARCH-154 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Settings preview already reflected the interface font and interface size,
but the editor font and editor size controls had no visual sample. A user
could save a valid editor choice without seeing how the editor typography
would look.

## Decision

Extend the existing `SettingsPreviewSurface` with an editor sample and a
localized editor-family/size line. Both the interface sample and editor sample
receive their `QFont` and point size through Qt API during the same preview
projection. `preview_stylesheet()` remains a color, border, and layout token
boundary only; it no longer interpolates any font name or dynamic font size.

The Settings dialog connects editor-font and editor-size changes to the same
refresh path already used by theme, accent, interface font/size, and locale.

## Invariants

1. Only presentation files change: `settings_preview.py`, `settings_dialog.py`,
   `i18n.py`, and `theme.py`.
2. `SettingsSnapshot`, `SettingsService`, font allowlists, `UserRole` values,
   persistence, post-save application, editor adapter behavior, and size
   ranges remain unchanged.
3. Preview is a one-way projection; it does not save, apply, mutate domain
   state, or introduce a second font policy.
4. `preview_stylesheet(colors)` has no font-name or dynamic-size parameters;
   font values are applied by `QFont` on the two named sample labels.
5. The new English/Chinese keys have matching named placeholders and are
   refreshed on locale changes.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation change. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Ramanujan the 5th / Luna max — initial `REVISE` for QSS/font
  interpolation risk; final `PASS` after the QFont-only correction.
- Independent review: Franklin the 5th / Luna max — initial `REVISE` for the
  existing interface sample's font interpolation; final `PASS` after both
  samples moved to QFont projection. The checkout has no Git baseline, so
  complete-diff proof remains limited.
- Parent review: `PASS` for refresh ordering, QFont/point-size projection,
  scoped QSS, localization placeholders, and unchanged settings ownership.
- Simplification assessment: `PASS`; one preview surface remains the single
  projection owner and the theme function loses unnecessary typography
  parameters.

## Verification target and limits

- Authorized evidence: AST/source-shape probes, QFont projection probe,
  refresh-wiring probe, scoped-QSS probe, localization-key/placeholder probe,
  compileall, Ruff, format, PyInstaller package build, package identity,
  project static checks, handoff checks, and expected release NO-GO evidence.
- Not proven: native popup/rendering, installed-font fallback metrics, DPI,
  accessibility, clean-machine behavior, cross-machine behavior, signing,
  installer/update, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active project policy.

