# Handoff: 2026-08-12-d229-startup-diagnostic-path-fail-open

| Field | Value |
|---|---|
| ID | `2026-08-12-d229-startup-diagnostic-path-fail-open` |
| Delivery / slice | `D229 / ARCH-211 Startup diagnostic path fail-open` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The early startup failure reporter can no longer lose the original exception
solely because the user-local diagnostic path, context collection, payload
construction, or log write fails. D228's useful context fallback remains.

## Scope and boundaries

- Changed implementation: `src/quillforge/__main__.py`.
- Kept the entry-boundary diagnostic owner and existing normal GUI path.
- No Qt, MainWindow, settings, locale, theme, document, or plugin behavior
  changed.
- No EXE/Qt launch was performed under the permanent no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` plus bounded `Boole the 6th / Luna max` window | Boundary and extensibility review |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Preserve actionable early failures |
| Developer 1 | `parent` | Entry-boundary implementation |
| Developer 2 | `parent` | Privacy and exception-boundary review |
| Test / QA | `parent` | Non-destructive source/package/handoff checks |

## Changed files and modules

- `src/quillforge/__main__.py`
- `docs/adr/0275-startup-diagnostic-path-fail-open.md`
- D229 parent and independent review records
- acceptance, delivery register, architecture, roadmap, task, and handoff
  index records

## Decisions and constraints

- Resolve the startup report path within an explicit fail-open boundary.
- Keep `execution_context: unavailable` when context collection fails.
- Protect payload construction and writes so the original exception remains the
  authoritative failure.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Review record

- Architecture role: `Boole the 6th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Independent role: `Confucius the 6th / Luna max` — `PASS` for the bounded
  D229 source review; native runtime is explicitly not covered.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Path fail-open probe | `PASS` | `D229-FAIL-OPEN-PATH-PROBE=PASS`. |
| Context fallback probe | `PASS` | `D229-CONTEXT-FALLBACK-PROBE=PASS`. |
| Compile | `PASS` | `D229-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D229-RUFF=PASS`. |
| Format | `PASS` | `D229-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D229-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D229-PACKAGE-BUILD-PS51=PASS`; intermediate hash `081930958CB6D74A6BE8585FDF014691A13D9BB4E625CDDAC05FC5A8234F85BC`. |
| PS7 package | `PASS` | `D229-PACKAGE-BUILD-PS7=PASS`; final candidate hash `2821DA9EBA3230B34364F57CA18F229914BEE4D6CD051B912CDA5BAD3218F093`. |
| Archive coverage | `PASS` | `D229-PACKAGE-ARCHIVE-PROBE=PASS`; 166 CArchive entries and required embedded modules/resources. |
| Package identity | `PASS` | `D229-PACKAGE-IDENTITY-PROBE=PASS`; root/dist match, 38,570,868 bytes, source `tree-sha256:b5f27e28db768b1ad325be9e63ef05aa796f5a39ca65a821d261e34a14f23e66`. |
| Project checks | `PASS` | `D229-CHECK-PS51=PASS` and `D229-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D229-HANDOFF-PS51/PS7=PASS`. |

## Public-source applicability

Python 3.12 public `pathlib`, `os`, `traceback`, and exception behavior are
the applicable first-party references. No dependency changed. Public
CloudWeGo material is engineering reference only; no private ByteDance
standard or certification claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

## Unrun checks and reason

- The packaged EXE, GUI/QApplication, native MessageBox rendering,
  accessibility, clean-machine, cross-machine, signing, installer/update,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  checks remain unrun under the permanent no-launch or external-authorization
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains expected no-go while
  artifact-bound runtime reports and external release gates remain open. The
  final PS7 verifier returned exit 1 as expected: 10 open gates and mechanical
  failures `packaged_report_artifact_match`,
  `interactive_startup_report_consistent`, and
  `startup_preflight_report_consistent`.

## Known risks and limits

- A broad `Exception` boundary is intentionally limited to the diagnostics
  helper; it prevents diagnostics from replacing the original startup error.
- Diagnostic paths can contain usernames or network roots; document contents
  and environment dumps remain excluded.
- Static/source/package evidence does not prove native startup success.

## Acceptance and evidence IDs

- Acceptance: `S279`.
- Evidence: `D229-FAIL-OPEN-PATH-PROBE=PASS`,
  `D229-CONTEXT-FALLBACK-PROBE=PASS`, `D229-COMPILEALL=PASS`,
  `D229-RUFF=PASS`, `D229-FORMAT=PASS`,
  `D229-PRESENTATION-AUDIT=PASS`, `D229-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D229-PACKAGE-IDENTITY-PROBE=PASS`, project/handoff checks `PASS`, and
  independent review `PASS`; release verifier expected `NO-GO` with 10 open
  gates.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: after explicit runtime authorization, reproduce packaged startup and
  attach the context-bearing log only if the native process still fails.

## Artifact information

D229 final PS7 artifact: `dist/QuillForge.exe` and `QuillForge.exe` are both
38,570,868 bytes with SHA-256
`2821DA9EBA3230B34364F57CA18F229914BEE4D6CD051B912CDA5BAD3218F093` and
source revision
`tree-sha256:b5f27e28db768b1ad325be9e63ef05aa796f5a39ca65a821d261e34a14f23e66`.

## Disposition

`accepted-with-limits`: D229 hardens the existing startup diagnostics boundary;
native runtime and enterprise release gates remain open.
