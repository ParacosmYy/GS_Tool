# ADR-0178: semantic state contrast closure

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-68 / ARCH-116

## Context

The centralized stylesheet reused semantic `success`, `danger`, and
`accent_alt` colors as text on their state backgrounds. Several supported
paper/sakura combinations did not meet the project's normal-text 4.5:1
threshold, especially working text on `pressed`, success text on
`success_bg`, and error text on `error_bg`. The resulting states could look
decorative while becoming difficult to read.

## Decision

Keep `presentation/theme.py` as the only owner of the QSS and derive three
local foreground endpoints in `_stylesheet()` with the existing
`_readable_foreground()` helper:

- `success_foreground` against `success_bg`;
- `working_foreground` against `pressed`; and
- `error_foreground` against `error_bg`.

Use those endpoints only for the existing success, working, and error text in
`statusMessage`, `statusPhase`, workspace/search status labels, and FindBar
feedback. Warning/gold handling, accent endpoint foregrounds, disabled/focus/
selected states, widget identities, signals, behavior, and application policy
remain unchanged.

## Alternatives rejected

- Changing the palette or semantic background tokens would alter every shell
  surface and exceed the narrow readability defect.
- Hard-coding one light/dark foreground would regress at least one supported
  theme family.
- Adding widget-local styles or a new feedback state would duplicate the
  centralized visual contract.

## Review and evidence

Gauss the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded review window. The independent review is
recorded separately. Parent review is `PASS`; simplification assessment is
`PASS` because the change reuses one existing contrast helper and introduces
only three local derived values in the canonical stylesheet boundary.

Authorized non-destructive evidence: `UI68-SEMANTIC-STATE-CONTRAST-PROBE=PASS:
36 pairs >= 4.5:1`, `UI68-QSS-STATE-FOREGROUND-PROBE=PASS`,
`UI68-COMPILEALL=PASS`, `UI68-RUFF=PASS`, `UI68-FORMAT=PASS`, project checks,
package identity, no-launch, traceability, and expected release NO-GO.
Native QSS rendering, font metrics, DPI, accessibility traversal, and
clean-machine/release-owner evidence remain unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation styling;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
