# Handoff: 2026-08-11-d223-startup-diagnostic

| Field | Value |
|---|---|
| ID | `2026-08-11-d223-startup-diagnostic` |
| Delivery / slice | `D223 / ARCH-206 No-window packaged startup diagnostic` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The QuillForge portable EXE now has an explicit no-window startup diagnostic
entry point. A support or QA operator can run
`QuillForge.exe --diagnose-startup --report <path>` and receive a JSON report
that distinguishes runtime imports, Qt/QScintilla availability, composition
loading, frozen Qt platform-plugin presence, and the authored icon. A passing
report returns `0`; a malformed, unwritable, or failed diagnostic returns `2`.

## Scope and boundaries

- Changed only `src/quillforge/app.py` for the implementation.
- The branch runs before `QApplication` construction and never opens the
  normal editor window or Qt event loop.
- The existing GUI dispatcher, composed runtime, themes, locale, settings,
  file dialogs, editor behavior, plugin policy, and one-file packaging shape
  remain unchanged.
- D222's user-local traceback boundary remains the fallback for unexpected
  exceptions in the normal windowed path.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `startup reliability` | Make the EXE failure diagnosable without a window |
| Developer | `parent` | Focused dispatcher implementation |
| QA | `parent` | Static, source diagnostic, archive, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/app.py`
- `README.md` (user-facing diagnostic command)
- Synchronized ADR, reviews, acceptance, delivery register, architecture,
  roadmap, task, release, index, and handoff records.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- The diagnostic is an executable preflight contract, not an application
  service and not a replacement for authorized runtime evidence.
- The active no-launch policy forbids GUI/QApplication and EXE startup by
  Codex; frozen execution remains user/QA-owned.
- No unit-test asset, mock, fixture, harness, or test-only file was created.

## Review record

- Architecture role: `Peirce the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Independent role: `Meitner the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop code.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Static source gate | `PASS` | `D223-STATIC-EXIT=0`; Ruff format/check and targeted compile passed. |
| Source diagnostic | `PASS` | `D223-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`; report JSON was written. |
| PS5.1 package | `PASS` | `D223-PACKAGE-BUILD-PS51=PASS`; candidate generated successfully. |
| PS7 package | `PASS` | `D223-PACKAGE-BUILD-PS7=PASS`; manifest is bound to this final candidate. |
| Archive coverage | `PASS` | `D223-PACKAGE-ARCHIVE-PROBE=PASS`; entry point, modules, QScintilla, `qwindows.dll`, and icon present. |
| Runtime archive coverage | `PASS` | `D224-RUNTIME-ARCHIVE-COVERAGE-PROBE=PASS`; 136 runtime modules reconciled, with only intentional `__main__` and packaging-only exclusions. |
| Package identity | `PASS` | Root/dist SHA `02A47AF05D54F19816C0A8DDD96FF8492484A5A8D8667638C8281193D63AA50C`, 38,566,961 bytes. |
| Record JSON | `PASS` | `D223-RECORD-JSON-PROBE=PASS`; `S274`, D223, and latest handoff index agree. |
| PS5.1 project check | `PASS` | `D223-CHECK-PS51=PASS`; notices, handoff, presentation audit, format, and checks passed. |
| PS7 project check | `PASS` | `D223-CHECK-PS7=PASS`; same checks passed under PowerShell 7. |
| Handoff verifier | `PASS` | `D223-VERIFY-HANDOFF-PS51/PS7=PASS`. |
| Release verifier | `NO-GO` | `D223-RELEASE-VERIFY-PS51/PS7-EXIT=1` with the three expected artifact-bound report consistency failures. |

## Public-source applicability

Python 3.12 standard-library `argparse`, `importlib`, `json`, and `pathlib`,
Qt's `QApplication` lifecycle, and PyInstaller runtime-information
documentation are the applicable public first-party references. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- Frozen `--diagnose-startup` execution, GUI/QApplication startup, native file
  dialog, screenshot, accessibility, clean-machine, cross-machine, signing,
  installer/update, legal, support, permission/disk-pressure, hard-power, and
  release-owner checks were not run under the active no-launch or external
  authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains an expected no-go until fresh
  authorized runtime reports and external release gates exist.

## Known risks and limits

- Static archive presence does not prove that the frozen process can load each
  binary on a clean machine.
- The diagnostic makes the startup failure class observable but does not by
  itself fix a machine-specific OS, permission, driver, or dependency issue.
- The report path is caller-selected and can fail for normal filesystem
  permission or disk-space reasons; that failure is reported with exit `2`.
- The checkout has no Git baseline; both delegated review windows returned
  `NO_CONCLUSION`.

## Acceptance and evidence IDs

- Acceptance: `S274`.
- Evidence: `D223-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`,
  `D223-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D223-PACKAGE-IDENTITY-PROBE=PASS`,
  `D223-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D223-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D223-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: QA / release engineering.
- Action: run the diagnostic on the exact distributed EXE in the authorized
  environment, attach the JSON report to the issue route, and then investigate
  the first failed check before requesting normal startup evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `02A47AF05D54F19816C0A8DDD96FF8492484A5A8D8667638C8281193D63AA50C` /
  `38,566,961` bytes
- Source revision: `tree-sha256:68028edd86daca972d87b75262a1e854455f5b64cb3751b685efeba764cdb01a`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: the EXE now exposes a no-window diagnostic contract
that can classify common early-startup failures; frozen runtime startup and
enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
