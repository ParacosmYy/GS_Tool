# D311 parent review — theme-aware shell separators

## Scope

Reviewed the centralized separator token resolution in
`presentation.theme._stylesheet()`, the `QMainWindow` and `QDockWidget`
selectors, hover treatment, the presentation audit, and the all-theme/accent
contrast probe.

## Findings

- PASS — the change stays inside the existing presentation stylesheet owner;
  no widget, layout, application, persistence, locale, or startup boundary is
  changed.
- PASS — the normal edge reuses the border token with a readable fallback
  across `surface_0`, `surface_1`, and `surface_2`.
- PASS — both main-window and dock separators share one rule and use the
  existing alternate accent only for hover feedback.
- PASS — the audit checks required QSS fragments and evaluates all 3 themes ×
  4 accents at the non-text contrast floor.
- PASS — formatting, compilation, Ruff, package identity, PE header, and
  frozen archive inventory passed.

## Simplification assessment

PASS. The smallest complete implementation is one derived token and one
shared selector pair in the existing stylesheet. Separate dock/window helper
styles or widget-level overrides would duplicate the contract and increase
coupling without adding behavior.

## Limits and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review returned `NO_CONCLUSION` after three
bounded Luna/max waits. Native EXE/Qt launch, rendering, accessibility, DPI,
alternate style engines, clean-machine behavior, and release gates remain
unverified. This is Python/PyQt6 desktop code; embedded vendor applicability
is N/A.

