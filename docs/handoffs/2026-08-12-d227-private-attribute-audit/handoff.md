# Handoff: 2026-08-12-d227-private-attribute-audit

| Field | Value |
|---|---|
| ID | `2026-08-12-d227-private-attribute-audit` |
| Delivery / slice | `D227 / ARCH-209 Private attribute declaration-aware audit` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The presentation startup contract audit now understands direct class-level
assignments and annotated fields when checking private `self` calls. This
reduces false alarms for intentionally declared presentation providers while
retaining the D226 protection against missing private accessors.

## Scope and boundaries

- Changed implementation: `scripts/audit_presentation_contracts.py`.
- Added `_class_attribute_names()` and merged its names into the existing
  private-call heuristic.
- No Qt, application, domain, infrastructure, plugin, locale, theme, file
  opening, or runtime behavior changed.
- No broad private-read audit was added.
- No Qt/QApplication or EXE launch was performed under the permanent
  no-launch boundary.

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- `docs/adr/0273-private-attribute-declaration-aware-audit.md`
- D227 parent and independent review records
- acceptance, delivery register, architecture, roadmap, task, release, and
  handoff index records

## Decisions and constraints

- The existing AST audit remains the single static contract owner.
- The rule is intentionally limited to direct private calls on `self` in
  top-level presentation classes; runtime callability and external inheritance
  remain outside its proof boundary.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Scope, integration, final review, release claims |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Reduce false alarms in the startup contract gate |
| Developer 1 | `parent` | Static contract implementation |
| Developer 2 | `parent` | Presentation/package boundary review |
| Test / QA | `parent` | Non-destructive source, package, and handoff checks |

## Review record

- Architecture role: `Dalton the 6th / Luna max` — `NO_CONCLUSION`; bounded
  window did not read enough source to conclude.
- Independent role: `Socrates the 6th / Luna max` — `NO_CONCLUSION`; bounded
  window could not obtain a repository diff.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Class declaration probe | `PASS` | `D227-CLASS-ATTRIBUTE-PROBE=PASS names=3`. |
| Presentation audit | `PASS` | `D227-PRESENTATION-AUDIT=PASS`. |
| Compile | `PASS` | `D227-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D227-RUFF=PASS`. |
| Format | `PASS` | `D227-FORMAT=PASS`. |
| PS5.1 package | `PASS` | `D227-PACKAGE-BUILD-PS51=PASS`; intermediate build completed before final PS7 candidate. |
| PS7 package | `PASS` | `D227-PACKAGE-BUILD-PS7=PASS`; final manifest binds the PS7 candidate. |
| Archive coverage | `PASS` | `D227-PACKAGE-ARCHIVE-PROBE=PASS`; entry point, Qt/QScintilla, platform plugin, icon, and required PYZ modules present. |
| Package identity | `PASS` | Root/dist SHA `8908DB5F54AFFD0B5688F76C3B3405C0338DE7BAF19A80FC260B750105745601`, 38,569,090 bytes, source revision `tree-sha256:2a73752948fcd703d3322e2ac1a39e3b381dd40ea9b604199165c5a12304213b`. |
| Project checks | `PASS` | `D227-CHECK-PS51=PASS`, `D227-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D227-HANDOFF-PS51/PS7=PASS`. |

## Public-source applicability

Python 3.12 public `ast` documentation and the existing uv/Ruff tooling are
the applicable engineering references. No dependency changed. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- The packaged EXE, GUI/QApplication, native menu/file-dialog rendering,
  accessibility, clean-machine, cross-machine, signing, installer/update,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  checks remain unrun under the permanent no-launch or external-authorization
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains expected no-go while
  artifact-bound runtime reports and external release gates remain open.

## Known risks and limits

- The static rule does not infer runtime callability, dynamic `setattr`,
  destructuring declarations, or private methods supplied by external base
  classes.
- The change reduces declaration-related false positives; it is not a type
  checker or proof of native startup.
- Both delegated review windows returned `NO_CONCLUSION`; parent review is
  the only review conclusion claimed.

## Acceptance and evidence IDs

- Acceptance: `S277`.
- Evidence: `D227-CLASS-ATTRIBUTE-PROBE=PASS names=3`,
  `D227-PRESENTATION-AUDIT=PASS`, `D227-COMPILEALL=PASS`, `D227-RUFF=PASS`,
  `D227-FORMAT=PASS`, `D227-PACKAGE-BUILD-PS51=PASS`,
  `D227-PACKAGE-BUILD-PS7=PASS`, `D227-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D227-PACKAGE-IDENTITY-PROBE=PASS`, `D227-CHECK-PS51=PASS`,
  `D227-CHECK-PS7=PASS`, `D227-HANDOFF-PS51/PS7=PASS`, and
  `D227-RELEASE-VERIFY-PS7=EXIT=1 expected NO-GO: 10 open gates; mechanical
  failures packaged_report_artifact_match, interactive_startup_report_consistent,
  startup_preflight_report_consistent`.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: use the rebuilt root `QuillForge.exe` only after explicit runtime
  authorization; if startup still fails, attach the refreshed user-local
  startup log.

## Artifact information

Artifact identity is bound to the final PS7 candidate: root and dist copies
match at SHA-256
`8908DB5F54AFFD0B5688F76C3B3405C0338DE7BAF19A80FC260B750105745601`,
38,569,090 bytes, with source revision
`tree-sha256:2a73752948fcd703d3322e2ac1a39e3b381dd40ea9b604199165c5a12304213b`.

## Disposition

`accepted-with-limits`: D227 improves the D226 static startup contract without
changing runtime behavior; native runtime and enterprise release gates remain
open.
