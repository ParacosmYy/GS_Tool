# ADR-0262: Semantic foregrounds for accent endpoints

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D215 / UI-116 / ARCH-200

## Context

The theme system already resolves readable foregrounds for the primary and
gold endpoints, but the primary button's hover state used the primary
`on_accent` value after switching its background to `accent_pink`. The current
12-combination static palette probe happened to pass, yet the ownership model
did not express that endpoint-specific contract clearly.

## Decision

Add one immutable `ThemeColors.on_accent_pink` token, resolved by the existing
`best_on_accent()` helper for every theme/accent pair. Use it only for the
existing `QPushButton#primaryAction:hover` selector. Keep the primary,
accent-alt, and gold foreground decisions in their existing owners and do not
introduce widget-local stylesheets, palette mutation, or runtime state.

## Invariants

1. Theme and accent IDs, settings, locale, fonts, motion, signals, and command
   behavior are unchanged.
2. All `ThemeColors` constructors remain complete and immutable.
3. The hover selector remains presentation-only and changes foreground
   resolution, not action semantics or layout.
4. `theme_tokens.py` remains Qt-free; `theme.py` remains the sole Qt/QSS
   projection owner.
5. The change is not evidence of native Qt rendering, accessibility, DPI, or
   release readiness.

## Public-source applicability and review

This is Python 3.12/PyQt6 presentation code. Qt's public [Style Sheets
Reference](https://doc.qt.io/qt-6/stylesheet-reference.html) is the applicable
first-party source for the selector/property boundary. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

The architecture role `Newton the 6th / Luna max` returned
`NO_CONCLUSION` after two bounded waits. The independent role
`Galileo the 6th / Luna max` also returned `NO_CONCLUSION` after two bounded
waits. Parent review is `PASS`; the behavior-preserving simplification
assessment is `PASS`.

## Verification and limits

- `D215-PINK-ENDPOINT-CONTRAST-PROBE=PASS combos=12`.
- `D215-QSS-PINK-WIRING-PROBE=PASS combos=12`.
- `D215-THEME-IMPORT-PROBE=PASS`.
- `python -m compileall -q src scripts` — PASS.
- Ruff check/format and presentation contract audit — PASS.
- Dual-shell package identity — PASS.

No GUI/QApplication, EXE launch, screenshot, native QSS painting, installed
font/DPI, accessibility, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was run. No unit-test
asset was created or run.
