# ADR-0154: Find close-affordance stability

- **Status:** accepted-with-limits; UI-63 bounded visual slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Find bar close control had only a transparent base and danger hover/focus
state. Its target size was implicit, and pressed/disabled states were not
authored, making the control visually unstable across native style metrics.

## Decision

Keep the existing `findClose` object and signal wiring, but give its scoped QSS
contract a 30px minimum height alongside the existing 30px width. Add token
driven pressed and disabled states, while retaining the existing danger hover
and focus treatment. The change stays in the central theme owner.

## Invariants

1. `FindBar` button identity, close signal, keyboard focus, locale, and layout
   ownership remain unchanged.
2. The close control no longer relies on a native style's implicit vertical
   metric for its target.
3. Hover/focus remain error-signaled; pressed uses the existing pressed token;
   disabled is muted and non-actionable.
4. No global button selector, icon, command, or application policy changes.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation styling. Embedded C/C++ and
manufacturer requirements are not applicable. Public CloudWeGo material is an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made.

## Review and simplification

- Architect: McClintock the 4th / Luna max; two bounded read-only waits timed
  out with `NO_CONCLUSION`.
- Independent review: Kierkegaard the 4th / Luna max; two bounded read-only
  waits timed out with `NO_CONCLUSION`.
- Parent review: PASS for selector scope, target sizing, and unchanged FindBar
  wiring.
- Simplification assessment: PASS. The smallest safe stylesheet-only change
  is retained; no new widget or state layer is introduced.

## Verification target and limits

- Authorized evidence: scoped QSS render/source probe, compileall, Ruff,
  format, presentation audit, package identity, and no-launch release checks.
- Native Qt style-engine rendering, accessibility inspection, DPI/font
  fallback, and runtime interaction remain unrun under the no-launch boundary.
