# ADR-0241: Close-guard pending feedback localization boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D193 / UI-103 / ARCH-179

## Context

When close was blocked by retained background work, the existing
`CloseGuardFeedbackCoordinator` generated a count-bearing English message.
`MessageSurface` already localized known close errors through
`presentation.i18n`, but the pending-count shape had no catalog entry or
dynamic matcher.

## Decision

Add the `error.wait_pending` English/Simplified Chinese catalog entry and one
strict matcher in `presentation.i18n`. The matcher preserves the bounded count,
translates the complete message, keeps English identity, and leaves unknown
shapes unchanged. Close-guard feedback remains the existing message boundary.

## Preserved invariants

- Close readiness classification, pending counts, TaskRunner lifecycle, session
  save, worker draining, and shutdown policy are unchanged.
- English (`en-US`) returns the original message unchanged.
- Counts are carried as bounded decimal/comma text without reinterpretation.
- Unknown message shapes remain visible for diagnosis instead of receiving a
  guessed translation.
- No second locale service or styling system is introduced.

## Review and applicability

The architecture consultation (`Raman the 6th / Luna max`) and independent
review (`Aquinas the 6th / Luna max`) both timed out within their bounded
windows; both are recorded as `NO_CONCLUSION`. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS`.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; the mandatory embedded
enterprise workflow is therefore recorded as not applicable to this source
slice. Public CloudWeGo material is an engineering reference only; this ADR
makes no private ByteDance standard, certification, or compliance claim.

## Evidence and limits

- `D193-CLOSE-GUARD-I18N-PROBE=PASS`
- `D193-COMPILE-RUFF-FORMAT=PASS`
- `D193-PRESENTATION-AUDIT=PASS`
- `D193-PACKAGE-BUILD=PASS`
- `D193-PACKAGE-IDENTITY-PROBE=PASS`

Native Qt rendering, accessibility-tree output, DPI, screenshot review,
GUI/EXE startup, clean-machine, cross-machine, legal, signing, installer,
updater, support, and release-owner evidence remain open. Release verification
remains `no-go` under the current no-launch and external-gate policy.
