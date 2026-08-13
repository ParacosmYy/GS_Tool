# D213 / UI-115 parent review: editor-shell brand edge

## Decision

`PASS` for the bounded centralized-QSS change, accepted with explicit native
rendering and release limits.

## Review evidence

- The selector remains scoped to `QWidget#editorShell`; child editor, tab, and
  FindBar state rules are not changed.
- The new declaration follows the existing neutral `border` declaration and
  precedes the existing radius, so the shell keeps its full boundary while
  exposing a two-pixel brand edge.
- `accent_pink` is an existing immutable token resolved for all three themes
  and four accents; no new color or contrast path is introduced.
- `EditorShellSurface` composition, `DocumentTabSurface`, editor palette,
  locale, motion, commands, and file activation remain untouched.

## Simplification assessment

`PASS`: one declaration in the existing central stylesheet is smaller and less
coupled than adding a wrapper widget, dynamic property, palette mutation, or
new visual-state coordinator.

## Limits

No GUI/QApplication, EXE launch, native QSS painting, screenshot,
accessibility, DPI, unit-test asset, or release gate was run. Both delegated
review windows returned `NO_CONCLUSION`; parent review is the only claimed
PASS review conclusion.
