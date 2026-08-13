# ADR-0249: Disabled menu state hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D201 / UI-106 / ARCH-187

## Context

The menu stylesheet applied the accent background from
`QMenu::item:selected` to disabled items, then only replaced the foreground
with `text_muted`. Across the 12 theme/accent projections, this made disabled
selected text sit on an accent background with a lowest measured contrast of
about 1.02:1. A checked disabled item had the same inheritance problem through
the checked selector.

## Decision

Keep menu chrome in `presentation.theme` and add one explicit disabled-state
override after the generic disabled rule. Both selected-disabled and
checked-disabled items use the subdued `surface_2` surface, a
`border_strong` left boundary, `text_muted`, and a semibold weight. The
override is presentation-only and preserves QAction state, menu IDs, labels,
shortcuts, callbacks, locale, and command ownership.

## Preserved invariants

- No menu action is enabled, disabled, checked, selected, reordered, or
  executed by this change; only its QSS projection changes.
- The normal, hover, pressed, checked, selected, and indicator rules remain in
  their existing central stylesheet owner.
- The subdued disabled surface prevents accent inheritance and retains a
  non-color left-edge distinction for keyboard/selection state.
- Theme token resolution, settings persistence, motion policy, and dependency
  direction remain unchanged.

## Review and applicability

The architecture consultation (`Ramanujan the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Harvey the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one selector block is the smallest fix for the measured inheritance bug.

This is a Python 3.12/PyQt6 presentation-only change. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable; the mandatory
embedded enterprise workflow is therefore not applicable to this source
slice. The applicable public first-party source is the Qt 6 Style Sheets
Reference, especially `QMenu` item selectors:
https://doc.qt.io/qt-6/stylesheet-reference.html. Public CloudWeGo material
is engineering reference only; no private ByteDance standard, certification,
or compliance claim is made.

## Evidence and limits

- `D201-MENU-DISABLED-QSS-PROBE=PASS combinations=12 selectors=2`
- `D201-DISABLED-CONTRAST-PROBE=PASS minimum=text_muted/surface_2>=4.5`
- `D201-COMPILE-RUFF-FORMAT=PASS`
- `D201-PACKAGE-BUILD-PS51=PASS`
- `D201-PACKAGE-BUILD-PS7=PASS`
- `D201-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QSS rendering,
accessibility tree, DPI, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was performed. Native Qt
selector specificity and visual metrics remain runtime limits.
