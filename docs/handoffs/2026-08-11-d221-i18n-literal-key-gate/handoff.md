# Handoff: 2026-08-11-d221-i18n-literal-key-gate

| Field | Value |
|---|---|
| ID | `2026-08-11-d221-i18n-literal-key-gate` |
| Delivery / slice | `D221 / ARCH-204 i18n literal-key static gate` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

Presentation code now has a persistent static check that prevents a literal
missing translation key from entering the codebase unnoticed. The existing
English/Chinese catalogs, dynamic keys, locale fallback, and runtime UI path
remain unchanged.

## Scope and boundaries

- Extended `scripts/audit_presentation_contracts.py` with a canonical English
  catalog comparison.
- Scanned literal `tr()` keys only; dynamic keys remain an explicit boundary.
- No translation, GUI, EXE, runtime, or release-gate behavior changed.

## Review record

- Architecture role: `Hegel the 6th / Luna max` — `NO_CONCLUSION` after bounded
  waits and closure.
- Independent role: `Heisenberg the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6/static tooling code.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `translation reliability` | Prevent literal catalog drift |
| Developer | `parent` | Focused AST audit extension |
| QA | `parent` | Static, package, and release-boundary checks |

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- Synchronized ADR, reviews, acceptance, delivery register, architecture,
  roadmap, task, release, index, and handoff records.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- The existing AST audit remains the only static gate owner; no runtime
  translation service or dynamic-key restriction was added.
- The current no-launch policy forbids GUI/QApplication and EXE startup by
  Codex; runtime locale confirmation remains user/QA-owned.
- No unit-test asset, mock, fixture, harness, or test-only file was created.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Literal-key probe | `PASS` | `D221-I18N-LITERAL-KEY-PROBE=PASS catalog_keys=262 missing=0 dynamic_keys=allowed`. |
| Compile | `PASS` | `D221-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D221-RUFF=PASS`. |
| Format | `PASS` | `D221-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D221-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D221-PACKAGE-BUILD-PS51=PASS`. |
| PS7 package | `PASS` | `D221-PACKAGE-BUILD-PS7=PASS`. |
| Package identity | `PASS` | SHA `812E3DE56A2521FDF1D7524EE0C8CB4D5A7ED143952B993E8D4C70BE853A9606`, 38,565,743 bytes. |

## Public-source applicability

Python 3.12 AST/static tooling applies. Python's public [AST
documentation](https://docs.python.org/3.12/library/ast.html) is the
applicable first-party reference. Public CloudWeGo material remains an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made. Embedded C/C++, MCU, RTOS, and manufacturer
requirements are not applicable.

## Unrun checks and reason

GUI/QApplication, EXE launch, native locale rendering, screenshot,
accessibility, clean-machine, cross-machine, signing, installer/update, legal,
support, permission/disk-pressure, hard-power, and release-owner checks were
not run under the active no-launch or external-authorization policy. Unit
tests, mocks, fixtures, harnesses, and test-only assets were not created or
run under project policy.

## Known risks and limits

- Dynamic translation keys remain outside the static proof boundary.
- Static catalog membership does not prove native font metrics, accessibility,
  runtime locale refresh, or release readiness.
- The checkout has no Git baseline; delegated architecture and independent
  windows did not conclude.

## Acceptance and evidence IDs

- Acceptance: `S272`.
- Evidence: `D221-I18N-LITERAL-KEY-PROBE=PASS catalog_keys=262 missing=0
  dynamic_keys=allowed`, `D221-PACKAGE-IDENTITY-PROBE=PASS`,
  `D221-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D221-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D221-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: QA / release engineering.
- Action: keep the static gate in `scripts/check.ps1` and validate dynamic
  locale paths during an authorized runtime review.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`.
- Historical D221 SHA-256 / size: `812E3DE56A2521FDF1D7524EE0C8CB4D5A7ED143952B993E8D4C70BE853A9606` /
  `38,565,743` bytes.
- Historical D221 source revision:
  `tree-sha256:8cde8ee2725e247666547751f130aaf2a95565666bbafd84bea75c00b35954c8`.

## Disposition

`accepted-with-limits`: literal translation-key drift is statically guarded;
dynamic-key correctness, native rendering, runtime, and external release
gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
