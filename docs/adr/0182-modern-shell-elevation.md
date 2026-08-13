# ADR-0182: modern shell elevation

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-70 / ARCH-120

## Context

The centralized stylesheet already exposed the required theme, accent, state,
font, and focus tokens, but several adjacent shell surfaces still read as one
flat plane. Command-bar hover could blend into the command bar, the document
tab rail and tabs used a weak surface ladder, and the workspace empty state
retained a dashed, legacy-looking treatment.

## Decision

Keep `presentation/theme.py` as the single visual owner and refine only the
existing QSS selectors:

- lift the command bar to `surface_2`, give its hover actions `surface_3`, and
  use the existing pink accent token as a small top edge;
- strengthen the editor shell and workspace dock frame while keeping the
  editor canvas on `surface_0`;
- make the document tab rail `surface_1`, tabs `surface_2`, and selected tabs
  `surface_3`;
- give the status bar and status rail distinct shell surfaces; and
- replace the workspace empty state's dashed border with a compact token-based
  card and accent edge.

No widget, object name, signal, layout, locale, font setting, motion, theme or
accent selection, semantic state, accessibility focus/disabled behavior, or
application policy changes are included.

## Alternatives rejected

- Introducing gradients, image assets, or a second styling system would make
  the visual contract harder to theme and maintain.
- Hard-coded colors would bypass the existing all-theme/accent token resolver.
- Reworking widget construction or layouts would expand a bounded visual polish
  slice into a behavioral change.

## Review and evidence

Bohr the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded window. Aquinas the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because the diff stays inside the existing centralized stylesheet and
reuses existing semantic tokens.

Authorized non-destructive evidence: `UI70-SHELL-HIERARCHY-PROBE=PASS: 12
theme/accent projections`, `UI70-HOVER-SEPARATION-PROBE=PASS`,
`UI70-SELECTION-AND-SEMANTIC-CONTRACT-PROBE=PASS`, compile/lint/format,
project checks, package identity, no-launch, traceability, and expected release
NO-GO. Native style-engine rendering, font/DPI metrics, accessibility tooling,
and runtime visual evidence remain unrun under the active authorization
boundary.

Public-source applicability is Python 3.12/PyQt6 presentation styling; no
embedded C/C++, MCU, RTOS, or manufacturer requirement applies. No external
vendor rule was used as a conformance claim. Public CloudWeGo material remains
an engineering reference only, not a private ByteDance standard or a
certification/compliance claim.
