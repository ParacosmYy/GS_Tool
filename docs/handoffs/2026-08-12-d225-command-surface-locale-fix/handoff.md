# Handoff: 2026-08-12-d225-command-surface-locale-fix

| Field | Value |
|---|---|
| ID | `2026-08-12-d225-command-surface-locale-fix` |
| Delivery / slice | `D225 / ARCH-207 Command-surface locale accessor startup fix` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The startup crash recorded for the supplied `QuillForge.exe` was fixed at its
root cause. `CommandSurface` now resolves locale through its existing provider
when creating menus, creating the toolbar, refreshing commands, and
retranslating. The rebuilt candidate is ready for the user's authorized
runtime check.

## Scope and boundaries

- Changed implementation: `src/quillforge/presentation/command_surface.py`.
- Added one `_locale()` method that delegates to `_locale_provider()`.
- Preserved menu/toolbar command order, callbacks, shortcuts, localization
  catalog, theme/icon refresh, and MainWindow composition ownership.
- No GUI/QApplication or EXE launch was performed by Codex under the permanent
  project no-launch boundary.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Root-cause decision, integration, final review, handoff |
| Project Manager | `parent` | Scope, dependency, risk, and evidence record |
| Product | `startup reliability` | Restore the editor shell for the supplied EXE |
| Developer | `parent` | One-method presentation fix |
| QA | `parent` | Log, source, compile, package, archive, and gate checks |

## Changed files and modules

- `src/quillforge/presentation/command_surface.py`
- Synchronized ADR, reviews, acceptance, delivery register, architecture,
  roadmap, task, release, handoff index, and this handoff.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- The provider remains the only locale dependency; no fallback or duplicate
  locale state was added.
- The project's permanent no-launch boundary remains active, so native EXE
  startup must be confirmed by the user or authorized QA.
- No unit-test asset, mock, fixture, harness, or test-only file was created.

## Review record

- Architecture role: `Volta the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Independent role: `Lorentz the 6th / Luna max` — `NO_CONCLUSION` after
  bounded waits and closure.
- Parent review: `PASS`.
- Simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements: not applicable;
  this is Python/PyQt6 desktop code.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Root-cause log | `PASS` | `D225-STARTUP-LOG-ROOT-CAUSE=PASS`; missing `CommandSurface._locale` identified at menu creation. |
| Source fix probe | `PASS` | `D225-LOCALE-STARTUP-FIX-PROBE=PASS`. |
| Module import | `PASS` | `D225-COMMAND-SURFACE-IMPORT=PASS`; no QApplication constructed. |
| Compile | `PASS` | `D225-COMPILEALL=PASS`. |
| Source diagnostic | `PASS` | `D225-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`. |
| PS5.1 package | `PASS` | `D225-PACKAGE-BUILD-PS51=PASS`. |
| PS7 package | `PASS` | `D225-PACKAGE-BUILD-PS7=PASS`; manifest is bound to this candidate. |
| Archive coverage | `PASS` | `D225-PACKAGE-ARCHIVE-PROBE=PASS`; entry point, app/composition, QScintilla, `qwindows.dll`, and icon present. |
| Package identity | `PASS` | Root/dist SHA `1B4536194F1D161DB33AA8604FA834412DAF57E4598748A09CB02842634EEDE8`, 38,569,401 bytes. |
| Project check | `PASS` | `D225-CHECK-PS51=PASS`, `D225-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D225-HANDOFF-PS51=PASS`, `D225-HANDOFF-PS7=PASS`. |

## Public-source applicability

Python 3.12 `typing.Callable`, existing PyQt6 `QMainWindow` projection, and
the project's Python/PyQt6 packaging documentation are applicable public
references. No dependency changed. Public CloudWeGo material remains an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made. Embedded C/C++, MCU, RTOS, and manufacturer
requirements are not applicable.

## Unrun checks and reason

- The supplied EXE, GUI/QApplication, native menu rendering, accessibility,
  clean-machine, cross-machine, signing, installer/update, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks were not run
  under the permanent project no-launch or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run under project policy.
- `scripts/verify_release_handoff.ps1` remains an expected no-go until fresh
  authorized runtime reports and external release gates exist.

## Known risks and limits

- The fix addresses the captured first startup exception; a subsequent
  environment-specific failure can only be classified by an authorized EXE
  run.
- Static packaging and source diagnostics do not prove native Qt window
  creation or clean-machine behavior.
- Both delegated review windows returned `NO_CONCLUSION`; parent review is the
  only review conclusion claimed.

## Acceptance and evidence IDs

- Acceptance: `S275`.
- Evidence: `D225-STARTUP-LOG-ROOT-CAUSE=PASS`,
  `D225-LOCALE-STARTUP-FIX-PROBE=PASS`,
  `D225-SOURCE-DIAGNOSTIC-PROBE=PASS exit=0 failed=0`,
  `D225-PACKAGE-ARCHIVE-PROBE=PASS`,
  `D225-PACKAGE-IDENTITY-PROBE=PASS`,
  `D225-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D225-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D225-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: replace the old EXE with the final PS7 candidate and launch it; if
  it still fails, attach the refreshed `%LOCALAPPDATA%\QuillForge\startup-error.log`
  or run `--diagnose-startup --report <path>`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `1B4536194F1D161DB33AA8604FA834412DAF57E4598748A09CB02842634EEDE8` /
  `38,569,401` bytes
- Source revision: `tree-sha256:d7c354212b31f99b1c0b263464bbd325a61546107edb31924aae6c904e688fe6`
- Root/dist identity: both paths match the final PS7 candidate.

## Disposition

`accepted-with-limits`: the captured startup root cause was fixed and the
candidate rebuilt; native runtime and enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
