# D201 parent review: disabled menu state hierarchy

## Decision

`PASS` for the bounded presentation-only QSS slice, accepted with explicit
native-rendering and release limits.

## Review evidence

- The defect is reproducible from the existing selector order: selected or
  checked menu items receive an accent/selection background, while the later
  generic disabled rule changes only the foreground.
- The new compound selectors appear after the generic disabled rule and cover
  selected-disabled, checked-disabled, and selected-checked-disabled states.
- The replacement surface is `surface_2`, whose `text_muted` foreground is at
  least 4.5:1 in all 12 theme/accent combinations.
- Only `src/quillforge/presentation/theme.py` changed in the source slice;
  QAction, command registry, menu projection, locale, settings, and motion
  ownership are untouched.

## Simplification assessment

`PASS`: one centralized selector block fixes the inheritance bug. A new
widget, dynamic property, token, palette branch, or menu subclass would add
ownership and behavior without improving this bounded outcome.

## Limits

No Qt runtime, GUI, accessibility, DPI, EXE, or test-only asset was launched or
created. Delegated architecture and independent windows returned
`NO_CONCLUSION`; parent review is the only PASS review conclusion claimed.
