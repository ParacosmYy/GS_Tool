# D217 / UI-117 parent review: toolbar brand anchor

## Decision

`PASS` for the bounded presentation-only brand anchor, accepted with explicit
native/runtime and release limits.

## Evidence

- `CommandSurface` inserts one non-interactive `QLabel#toolbarBrand` before the
  existing action loop and does not create a `QAction` or command ID.
- Existing toolbar action order, callback binding, shortcut behavior, context
  label, and locale owner remain unchanged.
- Retranslation updates the brand text, accessible name, and tooltip through
  the existing locale projection path.
- Centralized QSS uses `text_primary` on `surface_3`; the 12 theme/accent
  combinations retain a minimum measured contrast ratio of 11.16.
- The change remains confined to `presentation.command_surface` and the
  existing centralized `presentation.theme` stylesheet.

## Simplification assessment

`PASS`: a single existing toolbar label and one scoped QSS rule are the
smallest complete visual identity slice; a new component, command, state
service, or icon registry entry would add unnecessary ownership.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Native Qt rendering, font metrics, DPI, accessibility, GUI/EXE runtime, and
release evidence remain unrun.
