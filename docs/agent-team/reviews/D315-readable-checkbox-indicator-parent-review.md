# D315 parent review — readable checkbox indicator states

## Scope

Reviewed the shared `QCheckBox::indicator` normal/hover/checked/focus/
disabled hierarchy, Settings `behaviorToggle` role and tone projections, the
derived edge tokens, the 3-theme/4-accent contrast matrix, the source render
probe, and the rebuilt portable candidate.

## Findings

- PASS — the change remains in the centralized presentation stylesheet and
  applies to both ordinary checkboxes and the existing Settings behavior
  toggles; no check-state, signal, keyboard, persistence, locale, application,
  or startup behavior changed.
- PASS — checked indicators use readable pressed/hover surfaces so Qt's native
  check mark stays legible; the accent remains an explicit border with a
  token-derived fallback when its non-text contrast is insufficient.
- PASS — focus and checked-disabled selectors remain ordered and explicit;
  disabled indicators use the existing muted text/surface pair rather than a
  low-contrast accent fill.
- PASS — the static contract checks the source fragments and all 3 themes × 4
  accents at a 4.5 normal-text contrast floor. The offscreen render probe
  exercised 12 resolved styles without showing a window or entering an event
  loop.
- PASS — formatting, compilation, Ruff, presentation audit, source
  diagnostic, package identity, PE header, and frozen archive inventory
  passed.

## Simplification assessment

PASS. The smallest complete fix is to reuse the existing readable-edge helper
and change four shared indicator states. A custom SVG, resource system,
checkbox subclass, event handler, or per-settings control would duplicate
presentation ownership and risk changing native interaction without improving
the underlying contrast issue.

## Limits and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review returned `NO_CONCLUSION` after three
bounded Luna/max waits. Native EXE/Qt launch, pixel-level checkbox painting,
keyboard/accessibility output, DPI, alternate style engines, clean-machine
behavior, and release gates remain unverified. This is Python/PyQt6 desktop
code; embedded vendor applicability is N/A.

