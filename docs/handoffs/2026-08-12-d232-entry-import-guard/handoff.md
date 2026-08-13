# Handoff: 2026-08-12-d232-entry-import-guard

| Field | Value |
|---|---|
| ID | `2026-08-12-d232-entry-import-guard` |
| Delivery / slice | `D232 / ARCH-214 Guarded entry-point application import` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

An import failure in the early application entry boundary now reaches the
existing startup traceback, native fallback, and stderr diagnostics instead of
terminating before those handlers are defined. The public `main(argv)` entry
contract remains available.

## Scope and boundaries

- Changed implementation: `src/quillforge/__main__.py`.
- `app.main` is resolved inside the existing public `main(argv)` wrapper.
- Package-relative and direct-source import branches are preserved.
- No Qt, MainWindow, settings, locale, theme, document, plugin, or business
  behavior changed.
- No EXE/Qt launch was performed under the permanent no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` plus bounded `Rawls the 6th / Luna max` window | Entry-boundary architecture review |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Preserve actionable diagnostics for import failures |
| Developer 1 | `parent` | Entry-boundary implementation |
| Developer 2 | `parent` | Direct/bundled import compatibility review |
| Test / QA | `parent` | Non-destructive source/package/handoff checks |

## Changed files and modules

- `src/quillforge/__main__.py`
- `docs/adr/0278-entry-import-guard.md`
- D232 parent and independent review records
- acceptance, delivery register, architecture, roadmap, task, and handoff
  index records

## Decisions and constraints

- Keep `main(argv)` as the stable public entry contract.
- Move only the `app.main` import into the existing guarded call path.
- Preserve the package-relative and direct-source branches.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Review record

- Architecture role: `Rawls the 6th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Independent role: `Ptolemy the 6th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Public main contract probe | `PASS` | `D232-LAZY-MAIN-IMPORT-CONTRACT-PROBE=PASS`. |
| Import-failure guard probe | `PASS` | `D232-ENTRY-IMPORT-FAILURE-GUARD-PROBE=PASS`; simulated import failure reached diagnostics. |
| Source startup diagnostic | `PASS` | `D232-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`; QtCore, QScintilla, composition, and icon checks passed without QApplication. |
| Compile | `PASS` | `D232-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D232-RUFF=PASS`. |
| Presentation audit | `PASS` | `D232-PRESENTATION-AUDIT=PASS`. |
| Package build | `PASS` | PS5.1 hash `89C177E091810D8DE2A17BFE0207D0FCEDF8BAF52A7910839B476EC4FE548CE3`; PS7 final SHA `4161A707CA67AA739F2596DD1378111A7BAC7F979A95B22E3DF45114031EAAF8`; 38,571,175 bytes final. |
| Package archive | `PASS` | `D232-PACKAGE-ARCHIVE-PROBE=PASS entries=166 pyz=PYZ.pyz`; external `__main__`, Qt plugin/resource, and embedded `quillforge.app/composition` entries present. |
| Project checks | `PASS` | `D232-CHECK-PS51=PASS`; `D232-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D232-HANDOFF-PS51/PS7=PASS`. |
| Release verifier | `EXPECTED-NO-GO` | Exit 1; 10 open gates; mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`. |

## Public-source applicability

Python 3.12 public import/module execution and exception behavior are the
applicable references, with PyInstaller public one-file entry behavior as the
packaging reference. No dependency changed. Public CloudWeGo material is
engineering reference only; no private ByteDance standard or certification
claim is made. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
applicable.

## Unrun checks and reason

- The packaged EXE, GUI/QApplication, native MessageBox rendering,
  accessibility, clean-machine, cross-machine, signing, installer/update,
  legal, support, permission/disk-pressure, hard-power, and release-owner
  checks remain unrun under the permanent no-launch or external-authorization
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not created
  or run under project policy.
- `scripts/verify_release_handoff.ps1` remains no-go while artifact-bound
  runtime reports and external release gates remain open; the final run
  returned exit 1 with 10 open gates and the three mechanical consistency
  failures recorded above.

## Known risks and limits

- The lazy import is source/packaging-path verified; native Windows startup and
  actual import failure behavior in the packaged EXE remain unverified.
- Diagnostic paths can contain usernames or network roots; document contents
  and environment dumps remain excluded.
- Static/source/package evidence does not prove native startup success.

## Acceptance and evidence IDs

- Acceptance: `S282`.
- Evidence: `D232-LAZY-MAIN-IMPORT-CONTRACT-PROBE=PASS`,
  `D232-ENTRY-IMPORT-FAILURE-GUARD-PROBE=PASS`,
  `D232-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`, `D232-COMPILEALL=PASS`,
  `D232-RUFF=PASS`, `D232-PRESENTATION-AUDIT=PASS`,
  `D232-PACKAGE-BUILD-PS51=PASS`, `D232-PACKAGE-BUILD-PS7=PASS`,
  `D232-PACKAGE-ARCHIVE-PROBE=PASS`, `D232-PACKAGE-IDENTITY-PROBE=PASS`,
  `D232-CHECK-PS51=PASS`, `D232-CHECK-PS7=PASS`, and
  `D232-HANDOFF-PS51/PS7=PASS`.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: after explicit runtime authorization, reproduce packaged startup
  only if native evidence is required.

## Artifact information

The current PS7 manifest is the source of truth for the refreshed candidate:

- `dist/QuillForge.exe` and root `QuillForge.exe` are both `38,571,175`
  bytes with SHA-256
  `4161A707CA67AA739F2596DD1378111A7BAC7F979A95B22E3DF45114031EAAF8`.
- Source snapshot: `tree-sha256:46214d9f1e1a4aafcbbf1b6a240854bb12fd97b1afb7b2d3d2590f2ae819fadd`.
- Manifest: `dist/QuillForge.release.json`; package is unsigned, portable,
  and not an installer.

## Disposition

`accepted-with-limits`: D232 closes the unguarded entry-import diagnostic gap;
native runtime and enterprise release gates remain open.
