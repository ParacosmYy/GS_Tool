# ADR-0239: Dynamic Find/Replace status localization boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D191 / UI-101 / ARCH-177

## Context

The Find/Replace surface already routed status text through
`presentation.i18n.localize_message`. Exact messages were translated, but the
old prefix fallback turned `Replaced 1,024 matches` into a mixed-language
message and left the bounded Replace All limit outcome entirely in English.
The issue was visible in Simplified Chinese without changing the editor
operation itself.

## Decision

Keep localization ownership in `presentation.i18n` and add two strict,
count-bearing matchers for the existing English message shapes. Each matcher
delegates to the existing `find.status.replaced_count` or `find.status.limit`
catalog key, preserving the numeric text while translating the complete
message. Remove the generic `Replaced ` prefix fallback so an unknown shape is
not rendered as a misleading half-translation.

## Preserved invariants

- English (`en-US`) returns the original message unchanged.
- Existing exact messages, progress-prefix messages, recovery messages, and
  diagnostic details retain their current behavior.
- Counts are captured as bounded decimal/comma text and passed unchanged to
  the existing catalog formatter.
- No editor worker, Replace All policy, cancellation/rollback behavior,
  signal, status level, persistence, or MainWindow ownership changes.
- No second locale service or presentation style system is introduced.

## Review and applicability

The architecture consultation (`Linnaeus the 6th / Luna max`) and independent
review (`Ampere the 6th / Luna max`) both timed out within their bounded
windows; both are recorded as `NO_CONCLUSION`. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS`.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable; the mandatory embedded
enterprise workflow is therefore recorded as not applicable to this source
slice. Public CloudWeGo material is an engineering reference only; this ADR
makes no private ByteDance standard, certification, or compliance claim.

## Evidence and limits

- `D191-I18N-MATCHER-PROBE=PASS`
- `D191-COMPILE-RUFF-FORMAT=PASS`
- `D191-PRESENTATION-AUDIT=PASS`
- `D191-PACKAGE-BUILD=PASS`
- `D191-PACKAGE-IDENTITY-PROBE=PASS`

Native Qt rendering, accessibility-tree output, DPI, screenshot review,
GUI/EXE startup, clean-machine, cross-machine, legal, signing, installer,
updater, support, and release-owner evidence remain open. Release verification
remains `no-go` under the current no-launch and external-gate policy.
