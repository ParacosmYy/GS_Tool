# D309 parent review — Theme-aware Tooltip surface

## Scope

Reviewed the global QToolTip selector, ThemeColors token reuse, readable edge
fallback, typography inheritance, QSS scope, contrast matrix, static audit,
and package boundary.

## Findings

- PASS — the rule is centralized in `_stylesheet()` and applies consistently
  to D308 action hints and existing tooltips without changing event or locale
  flow.
- PASS — Tooltip text uses `text_primary` on `surface_3`; the edge resolver
  selects accent_alt only when it clears the non-text contrast floor.
- PASS — existing UI font family and style remain inherited; only a bounded
  relative size/weight is specified for hierarchy.
- PASS — no new ThemeColors field, service, dependency, or widget-specific
  stylesheet was introduced.
- PASS — format, compileall, Ruff, presentation audit, token matrix, package
  identity, PE header, and frozen archive checks pass.

## Simplification assessment

PASS. One selector and one local derived edge token are the smallest complete
change. A separate Tooltip surface class or token schema field would add
coupling without another consumer.

## Review status and limits

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded waits. Native Qt rendering, screen reader
behavior, DPI, clean-machine behavior, and release gates remain unverified.
No unit tests, mocks, fixtures, or harnesses were added or run.

## Public-source applicability

Qt QSS/Tooltip/palette behavior and WCAG 2.2 are public engineering
references. Embedded public-vendor applicability is N/A because this is
Python/PyQt6 desktop presentation code, not embedded C/C++ or firmware.
