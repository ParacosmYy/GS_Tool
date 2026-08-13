# D312 parent review — theme-aware scrollbars

## Scope

Reviewed the centralized scrollbar token derivation and QSS selectors in
`presentation.theme._stylesheet()`, the compact geometry, hidden arrow-line
controls, hover/pressed states, static source contract, and the all-theme/accent
contrast probe.

## Findings

- PASS — the change stays within the existing presentation stylesheet owner;
  no scrollbar model, editor engine, widget callback, application, settings,
  locale, persistence, or startup boundary changes.
- PASS — normal, hover, and pressed handles all use existing semantic tokens
  with a readable fallback across `surface_1` and `surface_2`.
- PASS — vertical and horizontal selectors share the same geometry and state
  hierarchy, while native range/page behavior remains untouched.
- PASS — the audit checks the required selectors and evaluates all 3 themes ×
  4 accents at the non-text contrast floor.
- PASS — formatting, compilation, Ruff, presentation audit, source diagnostic,
  package identity, PE header, and frozen archive inventory passed.

## Simplification assessment

PASS. One derived token group and one shared selector set are the smallest
complete implementation. Per-widget scrollbar overrides, a scrollbar service,
or new interaction state would duplicate presentation ownership and add
coupling without improving behavior.

## Limits and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review returned `NO_CONCLUSION` after three
bounded Luna/max waits. Native EXE/Qt launch, scrollbar painting/dragging,
accessibility, DPI, alternate style engines, clean-machine behavior, and
release gates remain unverified. This is Python/PyQt6 desktop code; embedded
vendor applicability is N/A.

