# Handoff: 2026-08-12-d235-startup-path-edge-routing

| Field | Value |
|---|---|
| ID | `2026-08-12-d235-startup-path-edge-routing` |
| Delivery / slice | `D235 / ARCH-216 Startup-path edge routing` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

Qt value arguments written with one or two leading dashes no longer get
misclassified as document paths. When a queued startup path cannot be admitted
because its workspace boundary is unavailable, QuillForge now reports a
warning instead of silently dropping the request; temporary busy and session
restore barriers continue to retry through the existing lifecycle.

## Scope and boundaries

- Changed `_takes_qt_value()` in `src/quillforge/application/desktop_launch.py`.
- Changed the non-temporary rejection branch in
  `src/quillforge/presentation/main_window.py`.
- Reused the existing parser, operation tracker, document/workspace admission,
  and notification boundaries.
- No settings, locale, theme, font, animation, document service, registry,
  installer, or OS file-association behavior changed.
- No EXE/Qt or registry operation was performed.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` plus bounded `Schrodinger the 7th / Luna max` window | Edge-case boundary review |
| Project Manager | `parent` | Evidence and risk record |
| Product | `startup reliability` | Preserve visible file-open feedback |
| Developer 1 | `parent` | Qt argument normalization |
| Developer 2 | `parent` | Startup admission warning branch |
| Test / QA | `parent` | Non-destructive source/package checks |

## Changed files and modules

- `src/quillforge/application/desktop_launch.py`
- `src/quillforge/presentation/main_window.py`
- D235 ADR, parent/independent reviews, acceptance, delivery register,
  architecture, roadmap, task, release, and handoff records

## Decisions and constraints

- Preserve original Qt option text while normalizing only dash spelling.
- Do not warn for a temporary busy/session-restore rejection; those remain
  retriable through existing completion callbacks.
- Warn and continue for a permanent/unavailable admission rejection.
- Keep portable `file_associations: not-configured`; packaging associations are
  explicit, opt-in, HKCU-only, and remain unexecuted.
- No unit-test assets, mocks, fixtures, harnesses, or worktrees were created or
  used.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Parser edge probe | `PASS` | `D235-DESKTOP-LAUNCH-PARSER-PROBE=PASS`; one/two-dash Qt options, dedupe, and explicit `--` path. |
| Compile | `PASS` | `D235-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D235-RUFF=PASS`. |
| Presentation audit | `PASS` | `D235-PRESENTATION-AUDIT=PASS`. |
| Source startup diagnostic | `PASS` | `D235-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`. |
| Package warning scope | `PASS` | `D235-PYINSTALLER-WARNING-SCOPE-PROBE=PASS`. |
| Package archive | `PASS` | `D235-PACKAGE-ARCHIVE-PROBE=PASS entries=166 pyz=PYZ.pyz`. |
| Embedded PYZ modules | `PASS` | `D235-PYZ-MODULE-PROBE=PASS modules=261`. |
| Source/artifact boundary | `PASS` | `D235-SOURCE-REVISION-BOUNDARY-PROBE=PASS`; associations remain not-configured. |
| Package identity | `PASS` | PS7 final SHA `CBAF4460C5162BE71A86881C032B8DE6835CFE923A5D5473F758A85388674A07`; 38,574,557 bytes; root/dist match. |
| Package build | `PASS` | PS5.1 `A1C84FF93A4F36DEB09612ED87A8BBD5DEE18241DBC5E26B4B2240D63F3B0F4B`; PS7 final as above. |
| Project checks | `PASS` | `D235-CHECK-PS51=PASS`; `D235-CHECK-PS7=PASS`. |
| Handoff verifier | `PASS` | `D235-HANDOFF-PS51=PASS`; `D235-HANDOFF-PS7=PASS`. |
| Release verifier | `EXPECTED-NO-GO` | `D235-RELEASE-VERIFY=EXPECTED-NO-GO`; 10 open gates plus artifact-bound report consistency failures. |

## Public-source applicability

Python 3.12 standard-library path/argument behavior, PyQt6 argument forwarding,
and PyInstaller one-file packaging are engineering references. No dependency
changed. Public CloudWeGo material is engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- Native EXE/QApplication, GUI rendering, Windows shell drag-and-drop, registry
  association, installer/update/rollback, clean-machine, cross-machine,
  signing, legal, support, and release-owner checks remain unrun under the
  active no-launch or external-authorization policy.
- Unit tests and test-only assets were not created or run.
- Release verification remains expected NO-GO while runtime reports and
  external release gates remain open.

## Known risks and limits

- Static/package evidence does not prove native Qt startup or shell behavior.
- A future installer must own any opt-in association registration and cleanup.
- Startup paths intentionally wait behind recovery/session restoration.

## Acceptance and evidence IDs

- Acceptance: `S284`.
- Evidence: `D235-DESKTOP-LAUNCH-PARSER-PROBE=PASS`,
  `D235-COMPILEALL=PASS`, `D235-RUFF=PASS`,
  `D235-PRESENTATION-AUDIT=PASS`, `D235-STARTUP-DIAGNOSTIC-SOURCE-PROBE=PASS`,
  `D235-PACKAGE-ARCHIVE-PROBE=PASS`, `D235-PYZ-MODULE-PROBE=PASS`,
  `D235-SOURCE-REVISION-BOUNDARY-PROBE=PASS`,
  `D235-PACKAGE-IDENTITY-PROBE=PASS`, and review/simplification records.

## Next owner and next action

- Owner: user / authorized QA for native EXE and file-association validation;
  architect for the next static iteration.
- Action: validate the refreshed EXE with a real file and, separately, the
  opt-in packaging script in a disposable authorized Windows environment.

## Artifact information

- `dist/QuillForge.exe` and root `QuillForge.exe`
- SHA-256: `CBAF4460C5162BE71A86881C032B8DE6835CFE923A5D5473F758A85388674A07`
- Size: `38,574,557` bytes
- Source revision: `tree-sha256:d6a54ba1d27c49eec900c54d5c3307bbe2d4e56a1cab407a1a63b6ead64e9a16`
- Unsigned portable candidate; file associations remain `not-configured`

## Disposition

`accepted-with-limits`: D235 closes the remaining startup-path argument and
admission-feedback edges while native runtime and enterprise release gates
remain open.
