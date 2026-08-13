# ADR-0234: Form-control highlight closure

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D185 / UI-96 / ARCH-172

## Decision

Use the existing centralized QSS in `presentation.theme` to make form-control
states easier to see:

- focused `QLineEdit`, `QSpinBox`, and `QComboBox` controls use the existing
  `ThemeColors.surface_hover` surface with the existing `accent_alt` border;
- ComboBox popup items expose normal, hover, and selected states;
- selected popup items reuse `selection`, `text_primary`, and an `accent_alt`
  left rail, with the existing emphasis weight.

The change reuses the current token resolver and object-name selectors. It does
not add a component library, state store, signal, settings, locale, font,
motion, layout, or application policy.

## Evidence and limits

- `D185-QSS-HIGHLIGHT-CONTRACT-PROBE=PASS combos=12`.
- `D185-COMPILE-RUFF-FORMAT=PASS`.
- Parent review and simplification assessment: `PASS`.
- Architect Hypatia the 5th / Luna max and independent Russell the 5th /
  Luna max windows returned `NO_CONCLUSION` after bounded waits.
- No GUI, native popup, EXE, screenshot, or unit-test asset was run/created.

This is Python/PyQt6 presentation work. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable. Public engineering guidance is
not a private ByteDance standard and does not establish certification or
compliance. Native QSS rendering, popup metrics, accessibility, DPI, runtime,
clean-machine, cross-machine, and release-owner evidence remain open.
