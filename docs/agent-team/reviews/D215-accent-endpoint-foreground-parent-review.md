# D215 / UI-116 / ARCH-200 parent review: accent endpoint foregrounds

## Decision

`PASS` for the bounded presentation-only token change, accepted with explicit
runtime and release limits.

## Evidence

- `ThemeColors` has a complete `on_accent_pink` field in all three base theme
  constants and in the centralized resolver.
- The existing `best_on_accent()` helper resolves the new token without a new
  color algorithm or mutable state.
- The only QSS consumer is the existing primary-button hover selector, whose
  background is `accent_pink`.
- All 12 theme/accent combinations pass the pink endpoint contrast probe and
  the generated-QSS wiring probe.
- `theme_tokens.py` remains Qt-free and `theme.py` remains the sole QSS owner.

## Simplification assessment

`PASS`: one semantic token and one existing selector consumer are smaller and
clearer than reusing a mismatched endpoint token, duplicating a foreground
literal, or introducing a style service.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Native QSS rendering, GUI/QApplication, EXE startup, accessibility, DPI,
font fallback, runtime interaction, and release gates remain unrun/open.
