# Handoff: 2026-08-12-d233-desktop-file-launch

| Field | Value |
|---|---|
| ID | `2026-08-12-d233-desktop-file-launch` |
| Delivery / slice | `D233 / ARCH-215 Desktop file-launch routing` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

`QuillForge.exe <file>` and Windows drag-and-drop onto the executable now
classify the file as a startup document and route it through the existing
asynchronous document-open boundary. Directory arguments route through the
existing workspace-open boundary. Multiple explicit paths remain ordered and
are opened serially after startup recovery/session restoration; missing paths
produce a warning instead of silently disappearing.

## Scope and boundaries

- Added the Qt-free `DesktopLaunchRequest` parser in
  `src/quillforge/application/desktop_launch.py`.
- `app.main` now separates Qt arguments from explicit startup paths.
- `DesktopRuntime` transfers immutable startup paths through composition.
- `MainWindow.open_startup_paths()` owns the recovery/session barrier and
  reuses existing document/workspace admission coordinators.
- No document service, workspace service, session schema, recovery policy,
  file picker, plugin lifecycle, or OS file-association registry changed.
- No EXE/Qt launch was performed under the permanent no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` plus bounded `Pascal the 7th / Luna max` window | Startup-path boundary review |
| Project Manager | `parent` | Milestone, risk, and evidence record |
| Product | `file-open reliability` | Ensure direct file launch reaches the editor |
| Developer 1 | `parent` | Parser and entry/composition wiring |
| Developer 2 | `parent` | MainWindow startup ordering and serial queue |
| Test / QA | `parent` | Non-destructive source/package/handoff checks |

## Changed files and modules

- `src/quillforge/application/desktop_launch.py`
- `src/quillforge/app.py`
- `src/quillforge/composition.py`
- `src/quillforge/presentation/main_window.py`
- D233 ADR, parent/independent review records, acceptance, delivery register,
  architecture, roadmap, task, release, and handoff-index records

## Decisions and constraints

- Keep path classification Qt-free and keep `QApplication` limited to Qt
  arguments.
- Transfer startup paths through the composition root, then open them through
  the existing async admission coordinators after recovery/session restore.
- Preserve ordered, single-flight startup opens and visible invalid-path
  warnings; do not add a second document/workspace service.
- Do not configure OS file associations in this portable candidate.
- No unit-test asset, mock, fixture, harness, or test-only file was created or
  run; no Git/Codex worktree was created or used.

## Review record

- Architecture role: `Pascal the 7th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Independent role: `Descartes the 7th / Luna max` — bounded window returned
  `NO_CONCLUSION` and was closed.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop tooling.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Parser behavior | `PASS` | `D233-DESKTOP-LAUNCH-PARSER-PROBE=PASS`; dedupe, `--`, and `-platform windows` coverage. |
| Source wiring | `PASS` | `D233-STARTUP-WIRING-SOURCE-PROBE=PASS`. |
| Source startup diagnostic | `PASS` | `D233-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`; no QApplication constructed. |
| Compile | `PASS` | `D233-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D233-RUFF=PASS`. |
| Presentation contract audit | `PASS` | `D233-PRESENTATION-AUDIT=PASS`. |
| PyInstaller warning scope | `PASS` | `D233-PYINSTALLER-WARNING-SCOPE-PROBE=PASS`. |
| Package archive | `PASS` | `D233-PACKAGE-ARCHIVE-PROBE=PASS entries=166 pyz=PYZ.pyz`. |
| Embedded PYZ modules | `PASS` | `D233-PYZ-MODULE-PROBE=PASS modules=261`; app, composition, and desktop-launch modules present. |
| Package identity | `PASS` | PS7 final hash `4DBA48F93ABB527BF83061F68A7A92A2CCB4646CA4FBD4D9CF76314848FB11A5`; 38,573,337 bytes; root/dist match. |
| Package build | `PASS` | PS5.1 `3CEE2CC5948481FB442A345E2CA1A77703978D846A618F6646B718EA240DD6E7`; PS7 final as above. |
| Project checks | `PASS` | `D233-CHECK-PS51=PASS`; `D233-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D233-HANDOFF-PS51=PASS`; `D233-HANDOFF-PS7=PASS`. |
| Release verifier | `EXPECTED-NO-GO` | `D233-RELEASE-VERIFY=EXPECTED-NO-GO`; exit 1 with 10 open gates and mechanical report-consistency failures. |

## Public-source applicability

Python 3.12 public `pathlib`, `collections.abc.Sequence`, and process
argument behavior are the applicable engineering references. PyInstaller's
public one-file entry behavior is the packaging reference. No dependency
changed. Public CloudWeGo material is engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- Native `QuillForge.exe` launch, `QApplication`, GUI rendering, Windows
  shell drag-and-drop, file-association registry behavior, accessibility, DPI,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks remain unrun
  under the permanent no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not created
  or run under project policy.
- `scripts/verify_release_handoff.ps1` remains expected no-go while current
  artifact-bound runtime reports and external release gates remain open.

## Known risks and limits

- Source and archive evidence prove packaging inclusion and routing structure,
  not native Windows startup or shell behavior.
- The portable candidate registers no OS file associations; an installer must
  own any future opt-in association policy.
- Startup paths are intentionally opened after recovery/session restoration,
  so a corrupt/slow recovery decision delays the explicit file open by design.

## Acceptance and evidence IDs

- Acceptance: `S283`.
- Evidence: `D233-DESKTOP-LAUNCH-PARSER-PROBE=PASS`,
  `D233-STARTUP-WIRING-SOURCE-PROBE=PASS`,
  `D233-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`, `D233-COMPILEALL=PASS`,
  `D233-RUFF=PASS`, `D233-PRESENTATION-AUDIT=PASS`,
  `D233-PACKAGE-ARCHIVE-PROBE=PASS`, `D233-PYZ-MODULE-PROBE=PASS`,
  `D233-PACKAGE-IDENTITY-PROBE=PASS`, `D233-SIMPLIFICATION-ASSESSMENT=PASS`,
  and the expected no-launch/release limits.

## Next owner and next action

- Owner: user / authorized QA for native EXE and Windows shell validation;
  architect for the next static iteration.
- Action: launch the refreshed candidate with one real text file and, if
  desired, multiple paths or a directory; verify native rendering and record
  the resulting artifact-bound runtime evidence only with explicit
  authorization.

## Artifact information

The current PS7 package candidate is `dist/QuillForge.exe` and root
`QuillForge.exe`, both 38,573,337 bytes with SHA-256
`4DBA48F93ABB527BF83061F68A7A92A2CCB4646CA4FBD4D9CF76314848FB11A5`.
The package manifest records source snapshot
`tree-sha256:c65b7dd81c9e3eaf14d4595edbd1910f5eb3854cde0aeeb7849448658a9400fb`.
The package is unsigned, portable, and not an installer; file associations
remain `not-configured`.

## Disposition

`accepted-with-limits`: D233 closes the missing explicit file/desktop-launch
routing boundary while native runtime and enterprise release gates remain
open.
