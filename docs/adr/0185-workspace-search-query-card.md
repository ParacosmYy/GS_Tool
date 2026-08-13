# ADR-0185: workspace-search query card

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-71 / ARCH-123

## Context

The workspace search dialog already had a themed root label, result list, and
feedback states, but its query controls were a bare horizontal layout. This
made the primary search intent harder to scan and left the dialog visually
flatter than the rest of the modern shell.

## Decision

Add one presentation-only `QFrame#workspaceSearchQueryCard` around the existing
query label, line edit, case-sensitivity control, Search button, and Cancel
button. Keep the existing signal connections and keyboard behavior unchanged.
Extend the centralized stylesheet with scoped token rules for the card, query
field, and case checkbox: existing surface, border, accent, focus, hover, and
font tokens remain the only visual inputs.

The results/diagnostics lists, status feedback, busy/cancel projection,
localization, theme/accent selection, motion, and application policy remain in
their existing owners.

## Alternatives rejected

- Adding a second style system or hard-coded colors would break the centralized
  theme contract.
- Rebuilding the search dialog layout would expand a visual hierarchy fix into
  a behavior change.
- Adding an icon/asset dependency is unnecessary for this bounded card-level
  distinction.

## Review and evidence

Carver the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded window. Carson the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion. No
child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because one existing-container wrapper and scoped QSS are the smallest
complete visual change.

Authorized non-destructive evidence: `UI71-WORKSPACE-SEARCH-QUERY-CARD-
PROBE=PASS: 12 theme/accent projections`,
`UI71-SEARCH-SIGNAL-PRESERVATION-PROBE=PASS`,
`UI71-QSS-SCOPE-PROBE=PASS`, compile/lint/format, project checks, package
identity, no-launch, traceability, and expected release NO-GO. Native
style-engine rendering, font/DPI metrics, accessibility tooling, and runtime
visual evidence remain unrun under the active authorization boundary.

Public-source applicability is Python 3.12/PyQt6 presentation styling; no
embedded C/C++, MCU, RTOS, or manufacturer requirement applies. No external
vendor rule was used as a conformance claim. Public CloudWeGo material remains
an engineering reference only, not a private ByteDance standard or a
certification/compliance claim.
