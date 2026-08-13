# Handoff: 2026-08-12-d228-startup-failure-context

| Field | Value |
|---|---|
| ID | `2026-08-12-d228-startup-failure-context` |
| Delivery / slice | `D228 / ARCH-210 Startup failure context record` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

When the portable EXE fails before Qt can show its normal window, the existing
startup log now identifies the producing executable, working directory,
frozen/runtime state, Python version, and bundle root. This makes stale-package
and wrong-launch-context failures distinguishable without changing normal GUI
startup.

## Scope and boundaries

- Changed implementation: `src/quillforge/__main__.py`.
- Added fixed path/runtime context to the existing user-local startup log.
- Context collection is fail-open and cannot replace the original exception.
- No Qt, MainWindow, settings, locale, theme, file-open, plugin, or business
  behavior changed.
- No EXE/Qt launch was performed under the permanent no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Scope, integration, final review, release claims |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Make early EXE failures actionable |
| Developer 1 | `parent` | Entry-boundary implementation |
| Developer 2 | `parent` | Diagnostic/privacy boundary review |
| Test / QA | `parent` | Non-destructive source, package, and handoff checks |

## Changed files and modules

- `src/quillforge/__main__.py`
- `docs/adr/0274-startup-failure-context-record.md`
- D228 parent and independent review records
- acceptance, delivery register, architecture, roadmap, task, release, and
  handoff index records

## Decisions and constraints

- Keep startup logging at the entry boundary; do not introduce Qt or a second
  diagnostics service.
- Record only fixed path/runtime fields, not arguments, environment dumps,
  settings, secrets, or document contents.
- Path and context collection must fail open so the original exception remains
  reportable.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Review record

- Architecture role: `Avicenna the 6th / Luna max` — `NO_CONCLUSION` after a
  bounded window; direction and risks were returned without a patch conclusion.
- Independent role: `Epicurus the 6th / Luna max` — `NO_CONCLUSION` after a
  bounded window and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Startup context probe | `PASS` | `D228-STARTUP-CONTEXT-PROBE=PASS fields=5`. |
| Compile | `PASS` | `D228-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D228-RUFF=PASS`. |
| Format | `PASS` | `D228-FORMAT=PASS` after final source formatting. |
| Presentation audit | `PASS` | `D228-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D228-PACKAGE-BUILD-PS51=PASS`; intermediate hash `42148012EFB022A6D0CFFE3E968533A428D30CB034A88CDF5D9F6C21B1D8B5C8`. |
| PS7 package | `PASS` | `D228-PACKAGE-BUILD-PS7=PASS`; final candidate hash `E7E361635C0E8B4003EA68AD743AF906B2094F4B37F8C036DB32EA62B6241CC7`. |
| Archive coverage | `PASS` | `D228-PACKAGE-ARCHIVE-PROBE=PASS`; 166 CArchive entries and embedded `quillforge.app` / `quillforge.composition`. |
| Package identity | `PASS` | `D228-PACKAGE-IDENTITY-PROBE=PASS`; root/dist match, 38,570,902 bytes, source `tree-sha256:c2e0126f0176cfe1dd7dbd1e044399dd1fbc89c889afb6ab00fca2c323e2f300`. |
| Project checks | `PASS` | `D228-CHECK-PS51=PASS` and `D228-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D228-HANDOFF-PS51/PS7=PASS`. |

## Public-source applicability

Python 3.12 public `pathlib`, `sys`, `traceback`, and exception behavior are
the applicable first-party engineering references. No dependency changed.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded C/
C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- The packaged EXE, GUI/QApplication, native MessageBox rendering,
  accessibility, clean-machine, cross-machine, signing, installer/update,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  checks remain unrun under the permanent no-launch or external-authorization
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains expected no-go while
  artifact-bound runtime reports and external release gates remain open; the
  final PS7 run returned exit 1 as expected with 10 open gates and mechanical
  failures `packaged_report_artifact_match`,
  `interactive_startup_report_consistent`, and
  `startup_preflight_report_consistent`.

## Known risks and limits

- Diagnostic paths can contain usernames or network roots; no document
  contents or environment dump is recorded.
- The static/source probe does not prove the packaged native startup path.
- The independent review returned `NO_CONCLUSION`; parent review is the only
  positive review conclusion claimed.

## Acceptance and evidence IDs

- Acceptance: `S278`.
- Evidence: `D228-STARTUP-CONTEXT-PROBE=PASS fields=5`,
  `D228-FAIL-OPEN-PATH-PROBE=PASS`, `D228-COMPILEALL=PASS`,
  `D228-RUFF=PASS`, `D228-FORMAT=PASS`, `D228-PRESENTATION-AUDIT=PASS`,
  `D228-PACKAGE-ARCHIVE-PROBE=PASS`, `D228-PACKAGE-IDENTITY-PROBE=PASS`,
  `D228-CHECK-PS51=PASS`, `D228-CHECK-PS7=PASS`,
  `D228-HANDOFF-PS51/PS7=PASS`, and the expected no-go release verifier
  result.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: after explicit runtime authorization, reproduce only the packaged
  EXE startup and attach the new context-bearing startup log if it fails.

## Artifact information

The final PS7 manifest is the source of truth: `dist/QuillForge.exe` and
`QuillForge.exe` are both 38,570,902 bytes with SHA-256
`E7E361635C0E8B4003EA68AD743AF906B2094F4B37F8C036DB32EA62B6241CC7` and
source revision
`tree-sha256:c2e0126f0176cfe1dd7dbd1e044399dd1fbc89c889afb6ab00fca2c323e2f300`.

## Disposition

`accepted-with-limits`: D228 makes early EXE failures actionable without
changing normal startup; native runtime and enterprise release gates remain
open.
