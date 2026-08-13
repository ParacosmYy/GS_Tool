# D211 / UI-113 parent review: workspace disabled-item hierarchy

## Decision

`PASS` for the bounded presentation-only change, accepted with explicit
native-rendering and release limits.

## Review evidence

- `WorkspacePanel._item_for()` still marks inaccessible entries and the
  overflow marker disabled; no item data, signal, activation, locale, or icon
  code changed.
- The new rule is scoped to `QTreeWidget#workspaceTree::item:disabled`, so
  ordinary `QListWidget` rows are unaffected.
- The existing selected-disabled selector has greater specificity and remains
  earlier in the workspace state block; the new ordinary-disabled rule does
  not erase its selected surface or boundary.
- The disabled tree-container rule remains distinct from row-level disabled
  styling, preserving the loading projection.
- The muted foreground on `surface_2` remains at least 4.5:1 across all
  3-theme × 4-accent token projections.

## Simplification assessment

`PASS`: one existing scoped QSS selector is the smallest complete fix. A
custom delegate, new workspace state service, or item-kind presentation model
would add coupling without improving the requested disabled-row hierarchy.

## Limits

No GUI/QApplication, EXE, native QSS/item rendering, screenshot,
accessibility, DPI, live filesystem refresh, test-only asset, or release gate
was run. Both delegated windows returned `NO_CONCLUSION`; parent review is the
only claimed PASS review conclusion.
