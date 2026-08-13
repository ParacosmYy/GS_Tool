# ADR-0256: Plugin disabled action hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D209 / UI-111 / ARCH-194

## Context

The Plugin Catalog and Plugin Status dialogs already disable governance and
lifecycle actions when the selected entry is invalid, untrusted, or already in
the requested state. Those buttons previously fell through to the generic
disabled button rule, so the dialogs did not preserve a clear visual boundary
between an unavailable policy action and ordinary dialog chrome.

## Decision

Add one scoped disabled rule in the existing centralized theme stylesheet for
the four real action identities: catalog approve/revoke and status enable/
disable. The disabled actions use `surface_2`, `border`, `border_strong`, and
`text_muted`, with a neutral left edge and semibold text. Enabled primary,
warning, hover, focus, and pressed states remain owned by their existing
generic action rules.

## Preserved invariants

- `PluginCatalogDialog._update_actions()` and
  `PluginStatusDialog._update_actions()` retain all enablement predicates and
  signal behavior.
- Plugin identity, trust, approval, execution, lifecycle, locale, list
  selection, and security policy remain unchanged.
- The rule is presentation-only and reuses existing `ThemeColors` tokens; no
  new widget, state service, or plugin-policy dependency is introduced.
- The disabled text contrast is at least 5.14 across all 3 themes × 4 accent
  projections.

## Review and applicability

The architecture consultation (`Averroes the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Goodall the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one grouped scoped QSS rule is the smallest complete affordance without
duplicating plugin state or action policy.

The applicable public first-party source is Qt's Style Sheets Reference,
which documents the `:disabled` pseudo-state:
https://doc.qt.io/qt-6/stylesheet-reference.html. This is a Python 3.12 /
PyQt6 presentation-only change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material is an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made.

## Evidence and limits

- `D209-PLUGIN-DISABLED-SOURCE-PROBE=PASS`
- `D209-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`
- `D209-COMPILEALL=PASS`
- `D209-RUFF=PASS`
- `D209-FORMAT=PASS`
- `D209-PRESENTATION-AUDIT=PASS`
- `D209-PACKAGE-BUILD-PS51=PASS`
- `D209-PACKAGE-BUILD-PS7=PASS`
- `D209-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native Qt painting,
accessibility tree, DPI, live plugin operation, clean-machine, cross-machine,
signing, installer, updater, legal, support, or release-owner evidence was
performed. No unit-test asset was created or run. Native selector parsing and
runtime action interleavings remain open.

