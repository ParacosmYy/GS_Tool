# ADR-0309: Static notification localization audit

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D270 / ARCH-243

## Context

The presentation status surface already routes notifications through
`localize_message`, but a new literal `notify("...")` call could silently
reintroduce English text in the Simplified Chinese shell. Existing checks
covered literal catalog keys, private self-calls, coordinator boundaries, and
notification severity, but not this user-visible localization invariant.

## Decision

Extend `scripts/audit_presentation_contracts.py` with one static AST rule.
It scans `src/quillforge/**/*.py` for calls whose method name is `notify` and
whose first argument is a literal string. ASCII-bearing literals are checked
against the existing Qt-free `localize_message(..., "zh-CN")` boundary; a
literal that remains unchanged is reported. Dynamic f-strings and non-string
expressions are intentionally left to the existing runtime projection and
dynamic-diagnostic review.

The rule remains in the existing audit owner. It adds no runtime behavior,
Qt dependency, catalog, test asset, or second localization mechanism.

## Boundaries and review

- Parent review: `PASS`.
- Architecture role `Mill the 7th / Luna max`: `NO_CONCLUSION` after a bounded
  wait and closure.
- Independent review `Euler the 7th / Luna max`: `NO_CONCLUSION` after a
  bounded wait and closure.
- Simplification assessment: `PASS`; the rule is one helper in the existing
  AST audit and does not duplicate notification behavior.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Public-source applicability

Python 3.12's first-party `ast` module and the project's existing Python
localization contract are the applicable references. Public CloudWeGo
material remains an engineering reference only; no private ByteDance
standard, certification, MISRA, ISO 26262, ASPICE, or embedded claim is made.

## Evidence and limits

- `D270-NOTIFICATION-LOCALIZATION=PASS static_ascii=52 translated=52 missing=0`.
- `D270-AUDIT-RULE=PASS exit_contract=0`.
- Presentation audit, compileall, Ruff, formatting, and `scripts/check.ps1`
  passed.
- The D270 portable candidate is x64/Windows GUI, manifest-bound, with root
  and `dist` SHA-256 `EDA5CFAF123D4201DDEAB41A153E2F3D39B50EE6860E9BC118F1DAF7655C49C0`.
- Native EXE/Qt startup, native dialogs, clean-machine behavior, signing,
  installer/update, registry, and release-owner evidence remain unrun under
  the active no-launch/non-destructive policy.

