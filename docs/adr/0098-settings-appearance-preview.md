# ADR-0098: Settings appearance preview

- **Status:** accepted-with-limits; D73 / UI-46 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The Settings dialog already persisted locale, theme, accent, interface font,
interface size, editor settings, and motion preference, but its appearance
choices were represented only by text controls. A user could not form a visual
judgment about a pending theme or typography choice before saving it.

## Decision

Add a presentation-only live preview card to `SettingsDialog`. The card projects
the currently selected theme, accent, interface font, and interface size without
calling `apply_theme`, writing settings, or changing the Save/Cancel contract.

Expose the existing canonical token resolver through `theme_colors()` and keep
the preview's scoped QSS in `preview_stylesheet()` inside `theme.py`. The dialog
reuses those tokens to show canvas, panel, selected, accent, and sample-text
states. Theme, accent, font, and size controls refresh the preview; locale
refresh reprojects its labels and metadata while retaining all selections.

## Invariants

1. `SettingsDialog` remains a presentation boundary; persistence, global theme
   application, editor projection, and notifications remain in their existing
   owners.
2. The preview is observational: closing with Cancel has no side effect, and
   only the existing explicit Save path returns a `SettingsSnapshot`.
3. The preview uses one token resolver and one scoped stylesheet contract; no
   duplicate color table or second settings model is introduced.
4. All three themes and four accents keep normal preview text and accent text at
   the recorded 4.5:1 contrast target in the static probe.
5. Native Qt rendering, font availability, DPI metrics, and accessibility are
   not inferred from static source evidence.

## Alternatives considered

- **Apply the theme immediately while editing:** rejected; it would change
  global state before Save and make Cancel surprising.
- **Add raster screenshots or third-party preview assets:** rejected; the
  existing token system already provides deterministic, scalable surfaces and
  avoids resource/licensing and packaging scope.
- **Duplicate color constants in `SettingsDialog`:** rejected; it would create
  a second visual source of truth and drift from the shell theme.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- `D73-PREVIEW-CONTRAST-PROBE=PASS` covers all 3 themes × 4 accents, required
  preview selectors, and pending font-family/size projection.
- Compileall, Ruff, format, package identity, handoff, and repository checks
  are recorded in the D73 handoff.
- The Architect and independent review windows returned no conclusion; no
  child PASS is claimed.
- Native `QApplication` startup, dialog rendering, interaction, accessibility,
  DPI, clean-machine, cross-machine, signing, legal, and release-owner
  evidence remain unrun or open under the active policy.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created.
