# Handoff: 2026-08-12-d226-private-self-call-contract-audit

| Field | Value |
|---|---|
| ID | `2026-08-12-d226-private-self-call-contract-audit` |
| Delivery / slice | `D226 / ARCH-208 Private self-call contract audit` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The existing presentation contract audit now catches the class of missing
private accessor that caused the supplied EXE's D225 startup failure, before a
future menu/toolbar projection reaches native startup. Existing application
behavior is unchanged.

## Scope and boundaries

- Changed implementation: `scripts/audit_presentation_contracts.py`.
- Added `_audit_private_self_calls()` to the existing AST contract audit.
- Allowed assigned callable/provider attributes remain valid; no runtime
  reflection or new dependency was added.
- No Qt/QApplication or EXE launch was performed under the permanent project
  no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Scope, integration, final review, release claims |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Prevent recurrence of early presentation startup failures |
| Developer 1 | `parent` | Static contract implementation |
| Developer 2 | `parent` | Presentation/package boundary review |
| Test / QA | `parent` | Non-destructive source, package, and handoff checks |

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- `docs/adr/0272-private-self-call-contract-audit.md`
- D226 parent/independent review records
- acceptance, delivery register, architecture, roadmap, task, release, and
  handoff records

## Decisions and constraints

- The existing AST audit remains the single static contract owner.
- The rule is intentionally limited to direct private calls on `self` in
  top-level presentation classes; dynamic reflection and external inheritance
  remain outside its proof boundary.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Review record

- Architecture role: `Lagrange the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Independent role: `Popper the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Private-call gate | `PASS` | `D226-PRIVATE-CALL-GATE-PROBE=PASS method=_locale calls=9 violations=0`. |
| Presentation audit | `PASS` | Existing audit including the new rule passed. |
| Compile | `PASS` | `D226-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D226-RUFF=PASS`. |
| Format | `PASS` | `D226-FORMAT=PASS`. |
| PS5.1 package | `PASS` | `D226-PACKAGE-BUILD-PS51=PASS`; build hash `7772843F8A15465EF356EEFDC52DE7508B5FDAA819AD9DFBB7C09947B18B6A67`. |
| PS7 package | `PASS` | `D226-PACKAGE-BUILD-PS7=PASS`; final manifest binds the PS7 candidate. |
| Archive coverage | `PASS` | `D226-PACKAGE-ARCHIVE-PROBE=PASS`; entry point, app/composition, QScintilla, qwindows.dll, and icon present. |
| Package identity | `PASS` | Root/dist SHA `EA0F1E1628340C2DE86DB598E923F84232E19D99CA32A9CA518AC7F1F2F4F3BD`, 38,566,742 bytes. |
| Project checks | `PASS` | `D226-CHECK-PS51=PASS`, `D226-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D226-HANDOFF-PS51/PS7=PASS`. |

## Public-source applicability

Python 3.12 public `ast` documentation and existing uv/Ruff tooling are the
applicable engineering references. No dependency changed. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

## Unrun checks and reason

- The packaged EXE, GUI/QApplication, native menu rendering, accessibility,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks remain unrun
  under the permanent no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains expected no-go while
  artifact-bound runtime reports and external release gates remain open.

## Known risks and limits

- The static rule does not infer runtime callability, dynamic `getattr`, or
  private methods supplied by external base classes.
- The rule prevents recurrence of this specific missing-accessor shape; it is
  not a general type-checker or proof of native startup.
- Both delegated review windows returned `NO_CONCLUSION`; parent review is the
  only review conclusion claimed.

## Acceptance and evidence IDs

- Acceptance: `S276`.
- Evidence: `D226-PRIVATE-CALL-GATE-PROBE=PASS method=_locale calls=9 violations=0`,
  `D226-COMPILEALL=PASS`, `D226-RUFF=PASS`, `D226-FORMAT=PASS`,
  `D226-PRESENTATION-AUDIT=PASS`, `D226-PACKAGE-BUILD-PS51=PASS`,
  `D226-PACKAGE-BUILD-PS7=PASS`, `D226-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D226-PACKAGE-IDENTITY-PROBE=PASS`,
  `D226-CHECK-PS51=PASS`, `D226-CHECK-PS7=PASS`,
  `D226-HANDOFF-PS51/PS7=PASS`,
  `D226-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D226-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D226-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: use the updated root `QuillForge.exe` for the authorized startup
  check; if it still fails, attach the refreshed user-local startup log.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `EA0F1E1628340C2DE86DB598E923F84232E19D99CA32A9CA518AC7F1F2F4F3BD` /
  `38,566,742` bytes
- Source revision: `tree-sha256:90570964b303bedd52c7d0803bc6ef8d75fa01ce133f1064c66929681a03d0f3`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: the D225 missing-accessor shape now has a static
presentation contract guard; native runtime and enterprise release gates
remain open.
