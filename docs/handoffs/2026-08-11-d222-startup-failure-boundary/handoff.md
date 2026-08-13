# Handoff: 2026-08-11-d222-startup-failure-boundary

| Field | Value |
|---|---|
| ID | `2026-08-11-d222-startup-failure-boundary` |
| Delivery / slice | `D222 / ARCH-205 Windowed startup-failure boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The windowed QuillForge EXE now has a visible and recoverable early-startup
failure path. If initialization fails before Qt can create the normal shell,
the entry point records a UTF-8 traceback at
`%LOCALAPPDATA%\QuillForge\startup-error.log` and attempts a native Windows
error message instead of silently disappearing.

## Scope and boundaries

- Changed only `src/quillforge/__main__.py`.
- Kept PyInstaller `console=False` and the existing one-file packaging shape.
- Preserved all application, domain, infrastructure, presentation, theme,
  settings, session, plugin, and diagnostic behavior.
- The startup log is a diagnostic aid, not proof that the Qt event loop opens.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `startup reliability` | Prevent silent windowed startup failure |
| Developer | `parent` | Focused entry-point implementation |
| QA | `parent` | Static, import, archive, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/__main__.py`
- Synchronized ADR, reviews, acceptance, delivery register, architecture,
  roadmap, task, release, index, and handoff records.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- The handler stays at the executable entry boundary and does not become an
  application service.
- The current no-launch policy forbids GUI/QApplication and EXE startup by
  Codex; runtime confirmation remains user/QA-owned.
- No unit-test asset, mock, fixture, harness, or test-only file was created.


## Review record

- Architecture role: `Russell the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Independent role: `Hooke the 6th / Luna max` — `NO_CONCLUSION` after bounded
  waits and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop code.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Startup report probe | `PASS` | `D222-STARTUP-REPORT-PROBE=PASS bytes=218`; temporary path, UTF-8 traceback, error type/message verified. |
| Source import smoke | `PASS` | 143 QuillForge modules imported without creating a Qt application. |
| Compile | `PASS` | `D222-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D222-RUFF=PASS`. |
| Format | `PASS` | `D222-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D222-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D222-PACKAGE-BUILD-PS51=PASS`; candidate SHA was individually verified. |
| PS7 package | `PASS` | `D222-PACKAGE-BUILD-PS7=PASS`; final manifest is bound to this candidate. |
| Package identity | `PASS` | Root/dist SHA `158178BF032BA94423D386380465B5B96E037EB38B1B4D80576FF1F4A044F6C8`, 38,565,695 bytes. |
| Archive coverage | `PASS` | Recursive archive contains `__main__`, QuillForge modules, QScintilla, `qwindows.dll`, and `assets\quillforge.ico`. |

## Public-source applicability

Python 3.12 standard-library entry-point code applies. Python's public
[built-in exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
and standard-library APIs are the applicable first-party references. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- GUI/QApplication, EXE launch, native message-box rendering, user-data ACLs,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks were not run
  under the active no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains an expected no-go because the
  current candidate is not backed by fresh authorized runtime reports and
  external release gates remain open.

## Known risks and limits

- A native message box depends on Windows user32 availability; the log remains
  the primary diagnostic fallback.
- The handler makes early failures observable but cannot prove that the
  underlying startup exception has been eliminated without authorized runtime
  evidence.
- The checkout has no Git baseline; delegated architecture and independent
  windows did not conclude.

## Acceptance and evidence IDs

- Acceptance: `S273`.
- Evidence: `D222-STARTUP-REPORT-PROBE=PASS bytes=218`,
  `D222-SOURCE-IMPORT-SMOKE=PASS modules=143`,
  `D222-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D222-PACKAGE-IDENTITY-PROBE=PASS`,
  `D222-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D222-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D222-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: QA / release engineering.
- Action: obtain an authorized runtime launch, inspect the generated startup
  log if needed, then refresh artifact-bound reports for the exact PS7 SHA.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `158178BF032BA94423D386380465B5B96E037EB38B1B4D80576FF1F4A044F6C8` /
  `38,565,695` bytes
- Source revision: `tree-sha256:0b790bfbc52c3288ff300b2179a2b4ae4558219b9125b975003cf4de164e169a`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: silent early startup failure now has a user-visible
diagnostic boundary; actual runtime startup and enterprise release gates
remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
