# ADR-0264: Toolbar brand anchor

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D217 / UI-117

## Context

The command rail already projects actions, a context capsule, and authored
icons, but its left edge begins with an action. The shell therefore lacks a
stable visual identity anchor when the user scans the first row. This is a
presentation-only clarity gap, not a command or application-state gap.

## Decision

Add one non-interactive `QLabel#toolbarBrand` before the existing toolbar
actions. It projects the localized product title with the existing `✦` brand
mark, accessible name, and tooltip. The centralized theme stylesheet owns its
surface, border, pink brand edge, typography, and spacing. The command list,
action count/order, callbacks, shortcuts, context label, locale owner, and
application policy remain unchanged.

## Boundaries

1. `CommandSurface` owns only the toolbar widget projection and retranslation
   of the brand label; it does not add an action or command ID.
2. `theme.py` remains the single QSS owner; no widget-local stylesheet,
   palette mutation, new theme token, or second styling system is introduced.
3. The brand uses `text_primary` on `surface_3`; the pink edge is decorative
   emphasis and is not used as a text foreground.
4. This slice does not claim native Qt rendering, font metrics, DPI, or
   runtime visual acceptance.

## Public-source applicability and review

This is Python 3.12/PyQt6 presentation code. Qt's public [Style Sheets
Reference](https://doc.qt.io/qt-6/stylesheet-reference.html) is the applicable
first-party source for the selector/property boundary. Public CloudWeGo
material is an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

The architecture role `Godel the 6th / Luna max` returned `NO_CONCLUSION`
after two bounded waits. The independent role `Darwin the 6th / Luna max`
returned `NO_CONCLUSION` after two bounded waits. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Verification and limits

- `D217-BRAND-ANCHOR-SOURCE-PROBE=PASS`.
- `D217-BRAND-CONTRAST-PROBE=PASS combos=12 min=11.16`.
- `D217-COMPILEALL=PASS`, `D217-RUFF=PASS`, `D217-FORMAT=PASS`, and
  `D217-PRESENTATION-AUDIT=PASS`.
- `D217-PACKAGE-BUILD-PS51=PASS`, `D217-PACKAGE-BUILD-PS7=PASS`, and
  `D217-PACKAGE-IDENTITY-PROBE=PASS`.

No GUI/QApplication, EXE launch, screenshot, native rendering, accessibility,
font fallback, DPI, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was run. No unit-test
asset was created or run.
