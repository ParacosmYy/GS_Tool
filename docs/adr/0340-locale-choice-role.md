# ADR-0340: Locale choice presentation role

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D304 / UI-123 / ARCH-274

## Context

The Settings language selector was the last appearance selector still styled
through concrete `settingsLanguage` rules. Theme/accent, typography, and
behavior controls already use semantic presentation roles. The language choice
therefore lacked the same explicit state hierarchy and was harder to extend
without adding another ID-specific selector group.

## Decision

Keep the existing `QComboBox` and assign the presentation-only dynamic property
`settingsRole="localeChoice"` immediately after its object identity and before
language item population. `presentation.theme` owns one locale-choice QSS
contract for normal, hover, focus, open, and disabled states. Its left edge is
resolved by `readable_edge_foreground_for_surfaces()` against the surface,
hover, and pressed backgrounds, with the existing accent-gold fallback.

No tone property or new widget is introduced: locale selection is an interface
choice and its semantic role is sufficient. The presentation audit guards
declaration order, state coverage, edge token use, and removal of the old
language ID selectors.

## Boundaries and compatibility

This slice does not change locale values, item data, current index, signals,
`set_locale()` behavior, labels, translations, settings schema, persistence,
keyboard routing, or application ownership. `settingsRole` is presentation-only
metadata and is not persisted.

## Evidence

- `D304-LOCALE-CHOICE-ROLE=PASS role=localeChoice item_data=current_data=preserved legacy_active_ids=0`
- `D304-LOCALE-CHOICE-CONTRACT=PASS currentIndexChanged=set_locale=locale_refresh=snapshot_preserved=1`
- `D304-QSS-MATRIX=PASS themes=3 accents=4 states=normal_hover_focus_on_disabled minimum_text=5.14 edge=3.62`
- Source startup and README file-open diagnostics passed without showing a
  window or entering the event loop.
- The rebuilt root/dist candidate has SHA-256
  `99891A57F00DC995EC7D8A4EE1110962E71470FC911AEB09A1B88BB8B1E9A2D5` and
  38,595,652 bytes.

## Review and applicability

The architecture consultation and independent code review each returned
`NO_CONCLUSION` after three bounded waits; no independent PASS is claimed.
Parent review is `PASS` and simplification assessment is `PASS`.

Qt Style Sheets and WCAG 2.2 SC 1.4.11 are public engineering references for
this Python 3.12/PyQt6 desktop UI. Embedded-vendor source applicability is
N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code changed. No
manufacturer, MISRA, ISO 26262, ASPICE, certification, or private corporate
standard claim is made.

## Limits

Native EXE/Qt launch, pixel rendering, focus/accessibility, DPI, clean-machine
behavior, real DLL loading, signing, installer, updater, and release-owner
gates remain unrun under the active non-destructive launch policy. Unit tests,
mocks, fixtures, and test harnesses were not created or run.
