# D198 parent review: menu affordance cohesion

## Decision

`PASS` for the bounded presentation-only slice, accepted with explicit native
rendering and external release limits.

## Review evidence

- The source change is confined to the centralized `presentation.theme` QSS
  projection.
- The existing menu/action projection in `CommandSurface` is untouched; no
  checkable action, callback, shortcut, locale, or command behavior changes.
- The indicator states use existing `ThemeColors` values and keep selected,
  checked, hover, and disabled state boundaries explicit.
- Static generation across all 12 theme/accent combinations passed, including
  accent text and indicator-boundary contrast checks.

## Simplification assessment

`PASS`: a single token-driven QSS block and one selected-checked refinement are
the smallest cohesive fix. A custom menu widget, image resource, or command
model change would add coupling without improving the bounded state contract.

## Limits

Native Qt menu painting, accessibility-tree behavior, DPI metrics, GUI/runtime
visual review, clean-machine, cross-machine, and release-owner evidence remain
unrun.
