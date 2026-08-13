# D209 parent review: plugin disabled action hierarchy

## Decision

`PASS` for the bounded presentation-only state projection, accepted with
explicit runtime and release limits.

## Review evidence

- The four selectors match the existing `primaryAction` and `warningAction`
  object names constructed by `PluginCatalogDialog` and `PluginStatusDialog`.
- The rule is scoped to the two plugin dialogs and appears after the generic
  enabled/hover/focus/disabled action rules, so disabled styling wins without
  changing available actions.
- `_update_actions()` predicates, signals, entry data, trust/approval policy,
  lifecycle state, and locale projection are unchanged.
- The neutral left edge communicates unavailable policy ownership while
  `text_muted` remains readable on `surface_2` in all 12 projections; minimum
  measured text contrast is 5.14.

## Simplification assessment

`PASS`: one grouped scoped rule is the smallest complete visual correction.
Adding a plugin state service, changing action predicates, or duplicating
button subclasses would increase coupling without improving the stated state
cue.

## Limits

No GUI/QApplication, EXE, native Qt painting, screenshot, accessibility, DPI,
live plugin action, test-only asset, or release gate was run. Both delegated
architecture and independent windows returned `NO_CONCLUSION`; parent review
is the only PASS review conclusion claimed.

