# ADR-0246: Menu affordance cohesion

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D198 / UI-105 / ARCH-184

## Context

The centralized menu stylesheet already distinguished ordinary, selected, and
checked menu items, but a checked item had no independent indicator contract.
That left future checkable commands dependent on a background-color change and
made the menu state less legible across the dark, light, and Sakura palettes.

## Decision

Keep `presentation.theme._stylesheet` as the sole owner of menu chrome. Add
token-driven `QMenu::indicator` states for normal, hover, checked, checked-hover,
and disabled items, plus an explicit selected-and-checked item rule. The change
does not make any existing `QAction` checkable and does not alter command
registration, shortcuts, signals, locale, or execution behavior.

## Preserved invariants

- Menu projection and command ownership remain in `CommandSurface` and
  `CommandRegistry`; no application or domain dependency is added.
- All colors and state boundaries are resolved from the existing `ThemeColors`
  token set; no image/resource pipeline or custom widget is introduced.
- Checked, selected, hover, and disabled states remain visually distinct while
  text foregrounds continue to use the existing readable accent tokens.
- The change is reversible by removing one bounded QSS block and one item-state
  refinement.

## Review and applicability

The architecture consultation (`Hume the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child PASS is claimed.
The independent review (`Locke the 6th / Luna max`) likewise returned no
conclusion after two bounded waits and was closed. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS` because the
presentation-only QSS block is the smallest change that closes the indicator
gap.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; the mandatory embedded
enterprise workflow is therefore not applicable to this source slice. Qt's
public Style Sheets Reference is the applicable first-party framework source.
Public CloudWeGo material is engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Evidence and limits

- `D198-MENU-AFFORDANCE-QSS-PROBE=PASS combinations=12 selectors=5`
- `D198-CONTRAST-PROBE=PASS combinations=12`
- `D198-COMPILE-RUFF-FORMAT=PASS`
- `D198-PRESENTATION-AUDIT=PASS`
- `D198-PACKAGE-BUILD-PS51=PASS`
- `D198-PACKAGE-BUILD-PS7=PASS`
- `D198-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QSS rendering, or
unit-test asset was created or run. Native menu metrics/accessibility, DPI,
clean-machine, cross-machine, signing, installer, updater, legal, support,
permission/disk-pressure, hard-power, and release-owner evidence remain open.
