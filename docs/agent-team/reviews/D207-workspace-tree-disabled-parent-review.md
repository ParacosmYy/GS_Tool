# D207 parent review: workspace tree disabled state

## Decision

`PASS` for the bounded presentation-only state projection, accepted with
explicit runtime and release limits.

## Review evidence

- The new selector is scoped to the existing workspace tree object and is
  ordered after its normal/focus rules.
- `WorkspacePanel.set_loading()` disables the visible populated tree during
  an existing-directory refresh; the source behavior and tree data policy are
  unchanged.
- The projection reuses `surface_2`, `border`, and `text_muted`, removing the
  actionable container surface without changing row selection semantics.
- Existing `::item:selected:disabled` remains intact for row-level state and
  the container rule does not duplicate item styling.
- Static token projection reaches 12 theme/accent combinations with a minimum
  disabled text/surface contrast of 5.14.

## Simplification assessment

`PASS`: one container-level scoped rule is smaller than duplicating item rules
or introducing a loading-state presenter/service.

## Limits

No GUI/QApplication, EXE, native QSS painting, screenshot, accessibility,
DPI, live provider refresh, test-only asset, or release gate was run.
Delegated architecture and independent review windows returned
`NO_CONCLUSION`; parent review is the only PASS review conclusion claimed.

