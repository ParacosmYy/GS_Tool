# Handoff: 2026-08-12-d230-startup-fallback-display-fail-open

| Field | Value |
|---|---|
| ID | `2026-08-12-d230-startup-fallback-display-fail-open` |
| Delivery / slice | `D230 / ARCH-212 Startup fallback display fail-open` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

When QuillForge fails before Qt can show its normal shell, a broken exception
string, native MessageBox, or stderr channel no longer replaces the original
startup failure. Ordinary failures retain the existing human-readable native
fallback behavior.

## Scope and boundaries

- Changed implementation: `src/quillforge/__main__.py`.
- Kept fallback presentation at the existing entry boundary.
- No Qt, MainWindow, settings, locale, theme, document, plugin, or business
  behavior changed.
- No EXE/Qt launch was performed under the permanent no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` plus bounded `Copernicus the 6th / Luna max` window | Boundary and compatibility review |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `startup reliability` | Preserve actionable early failures |
| Developer 1 | `parent` | Entry-boundary implementation |
| Developer 2 | `parent` | Display/exception safety review |
| Test / QA | `parent` | Non-destructive source/package/handoff checks |

## Changed files and modules

- `src/quillforge/__main__.py`
- `docs/adr/0276-startup-fallback-display-fail-open.md`
- D230 parent and independent review records
- acceptance, delivery register, architecture, roadmap, task, and handoff
  index records

## Decisions and constraints

- Keep ordinary startup failure text and the native MessageBox contract
  unchanged when those channels work.
- Make exception stringification, native display, and stderr reporting
  best-effort so the original exception remains authoritative.
- Keep broad catches limited to the diagnostics-only entry boundary.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run.
- No Git/Codex worktree was created or used.

## Review record

- Architecture role: `Copernicus the 6th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Independent role: `Mendel the 6th / Luna max` — `PASS` for the bounded static
  source review; native runtime is explicitly not covered.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Error-string fail-open probe | `PASS` | `D230-ERROR-STRING-FAIL-OPEN-PROBE=PASS`. |
| MessageBox fail-open probe | `PASS` | `D230-MESSAGEBOX-FAIL-OPEN-PROBE=PASS`. |
| Compile | `PASS` | `D230-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D230-RUFF=PASS`. |
| Format | `PASS` | `D230-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D230-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D230-PACKAGE-BUILD-PS51=PASS`; intermediate hash `4C399CCA5151D16743CE1DD11251ECCEFD342C3CD9BA0F9236A36911A2F164C0`. |
| PS7 package | `PASS` | `D230-PACKAGE-BUILD-PS7=PASS`; final candidate hash `40BBE341E630C390DB45E6D2C081773176752FD0FB6EA91C013F42B66AF0DF98`. |
| Archive coverage | `PASS` | `D230-PACKAGE-ARCHIVE-PROBE=PASS`; 166 CArchive entries and required embedded modules/resources. |
| Package identity | `PASS` | `D230-PACKAGE-IDENTITY-PROBE=PASS`; root/dist match, 38,569,926 bytes, source `tree-sha256:1578b1c774d74b94b9689359e330260b19f5e1aeace1fee03fde7be45ca2c75b`. |
| Project checks | `PASS` | `D230-CHECK-PS51=PASS` and `D230-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D230-HANDOFF-PS51/PS7=PASS`. |

## Public-source applicability

Python 3.12 public built-in exception, `ctypes`, `sys`, and stderr behavior are
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

- Broad catches are intentionally limited to fallback diagnostics; they do not
  suppress the final `SystemExit(1)` from the entry boundary.
- Diagnostic paths can contain usernames or network roots; document contents
  and environment dumps remain excluded.
- Static/source/package evidence does not prove native startup success.

## Acceptance and evidence IDs

- Acceptance: `S280`.
- Evidence: `D230-ERROR-STRING-FAIL-OPEN-PROBE=PASS`,
  `D230-MESSAGEBOX-FAIL-OPEN-PROBE=PASS`, `D230-COMPILEALL=PASS`,
  `D230-RUFF=PASS`, `D230-FORMAT=PASS`, and
  `D230-PRESENTATION-AUDIT=PASS`, `D230-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D230-PACKAGE-IDENTITY-PROBE=PASS`, project/handoff checks `PASS`, and
  independent review `PASS`; release verifier expected `NO-GO` with 10 open
  gates.

## Next owner and next action

- Owner: user / authorized QA for the runtime gate; architect for the next
  static iteration.
- Action: after explicit runtime authorization, reproduce packaged startup
  only if native evidence is required.

## Artifact information

D230 final PS7 artifact: `dist/QuillForge.exe` and `QuillForge.exe` are both
38,569,926 bytes with SHA-256
`40BBE341E630C390DB45E6D2C081773176752FD0FB6EA91C013F42B66AF0DF98` and
source revision
`tree-sha256:1578b1c774d74b94b9689359e330260b19f5e1aeace1fee03fde7be45ca2c75b`.

## Disposition

`accepted-with-limits`: D230 hardens the existing fallback display boundary;
native runtime and enterprise release gates remain open.
