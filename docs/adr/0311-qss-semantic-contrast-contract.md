# ADR-0311: QSS semantic contrast contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D275 / ARCH-245

## Context

D273 proved the base theme and editor semantic tokens across all supported
theme/accent choices, but the runtime stylesheet still assembled several
derived foregrounds locally. That left a gap between the audited token matrix
and the actual warning, success, working, error, selection, and accent-alt QSS
surfaces behind the reported 砂金 readability regression.

## Decision

Add the framework-neutral `QssForegroundTokens` value object and
`qss_foreground_tokens()` resolver to `presentation.theme_tokens`. The resolver
owns the existing contrast decisions for accent-alt text/fill, selection,
warning, success, working, and error states. `theme.py` consumes the resolver
for palette and QSS projection, and `audit_presentation_contracts.py` consumes
the same object for its regression gate.

The change does not alter widget signals, settings, theme identifiers, QSS
selectors, editor behavior, or application policy. It removes duplicated
foreground assembly rather than introducing a second visual policy.

## Review and boundaries

- Parent review: `PASS`.
- Simplification assessment: `PASS`; one immutable resolver is shared by
  runtime projection and the audit.
- Architecture role `Leibniz the 7th / Luna max`: `NO_CONCLUSION` after a
  bounded wait and safe closure.
- Independent review `Planck the 7th / Luna max`: `NO_CONCLUSION` after a
  bounded wait and safe closure.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable.

## Public-source applicability

Python 3.12 first-party `dataclasses`/typing behavior and the existing
framework-neutral theme-token contract are applicable references. No private
ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or embedded claim
is made.

## Evidence and limits

- `D275-QSS-CONTRAST=PASS checks=84 failures=0`.
- Presentation audit, compileall, Ruff, formatting, and `scripts/check.ps1`
  passed.
- PE is Windows x64 GUI; Qt/QScintilla/platform/style/icon runtime entries are
  present in the frozen archive.
- Current candidate SHA-256 is
  `CBDA1FDADCF25C143D0B24F36230DB9B5194722F63BBEC87670E4DA29340C944`,
  38,583,820 bytes; source revision is
  `tree-sha256:bbba300cd7a84d7ac7e06e3aff10c6dd028d8d9304ecb97cecbef54075a5c99d`.
- Native EXE/Qt startup, native rendering, accessibility, clean-machine
  behavior, signing, installer/update, registry, and release-owner evidence
  remain unrun.
