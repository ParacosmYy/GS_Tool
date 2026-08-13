# ADR-0179: settings-dialog card hierarchy

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-69 / ARCH-117

## Context

The settings dialog already had semantic object names and centralized QSS, but
the dialog canvas, two settings groups, preview, and action row shared nearby
surface values. That reduced visual grouping and made the modern/kawaii
appearance feel flat even though the controls and settings behavior were
complete.

## Decision

Keep the existing `theme.py` stylesheet as the sole visual owner and refine
only the settings dialog hierarchy:

- use `surface_0` for the settings dialog canvas;
- give the appearance group a `surface_2` card with a stronger boundary and
  cyan/alternate accent rail;
- give the editor group a quieter `surface_1` card with a rose accent rail;
- give the preview surface a distinct `surface_3` card with the primary accent
  rail; and
- give the settings `QDialogButtonBox#dialogActions` a bounded action-card
  background, border, and padding.

Existing object names, form layout, widget order, button roles, focus states,
settings persistence, locale/font/motion behavior, and application policy are
unchanged.

## Alternatives rejected

- Adding new widgets or reparenting the settings form would alter layout and
  accessibility ownership for a visual-only defect.
- Applying one global background to every group would preserve the flat
  hierarchy.
- Widget-local stylesheets would duplicate the central theme boundary.

## Review and evidence

Feynman the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. The independent
review is recorded separately. Parent review is `PASS`; simplification
assessment is `PASS` because the slice changes only existing QSS selectors and
introduces no component, state, or policy abstraction.

Authorized non-destructive evidence: `UI69-DIALOG-CARD-HIERARCHY-PROBE=PASS:
12 theme/accent projections`, `UI69-DIALOG-QSS-SCOPE-PROBE=PASS`,
`UI69-COMPILEALL=PASS`, `UI69-RUFF=PASS`, `UI69-FORMAT=PASS`, project checks,
package identity, no-launch, traceability, and expected release NO-GO. Native
QSS rendering, font metrics, DPI, accessibility traversal, and release-owner
evidence remain unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation styling;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
