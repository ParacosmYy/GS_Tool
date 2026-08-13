# Handoff: 2026-08-11-d220-main-shell-low-noise

| Field | Value |
|---|---|
| ID | `2026-08-11-d220-main-shell-low-noise` |
| Delivery / slice | `D220 / UI-118 Main-shell low-noise visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The main QuillForge workspace now has a calmer, more modern visual hierarchy:
the canvas is flat, the editor stage is lighter, the command rail is a compact
surface, and the document-tab rail no longer reads as a nested rounded card.
Theme/accent selection and all existing interaction states remain owned by the
same centralized stylesheet.

## Scope and boundaries

- Refined only `QMainWindow#mainWindow`, `QWidget#editorShell`,
  `QToolBar#commandBar`, and `QTabBar#documentTabBar` in `theme.py`.
- Removed shell gradients, reduced editor/toolbar radii, and flattened the
  document-tab rail using existing surface/border/accent tokens.
- Preserved object names, actions, signals, layout, locale, fonts, motion,
  theme resolution, icon projection, and normal/hover/pressed/focus/checked/
  selected/disabled states.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Boundary decision, integration, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `visual hierarchy outcome` | Modern, readable workspace surface |
| Developer | `parent` | Focused centralized-QSS refinement |
| QA | `parent` | QSS, contrast, static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/presentation/theme.py`
- Synchronized ADR, reviews, acceptance, register, roadmap, architecture,
  task, release, index, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Architecture role: `Halley the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Independent role: `Hilbert the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Evidence | Result | Notes |
|---|---|---|
| Main-shell QSS/contrast probe | `PASS` | `D220-MAIN-SHELL-QSS-PROBE=PASS selectors=4 combos=12 gradients=0`. |
| Compile | `PASS` | `D220-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D220-RUFF=PASS`. |
| Format | `PASS` | `D220-FORMAT=PASS`. |
| Presentation audit | `PASS` | `D220-PRESENTATION-AUDIT=PASS`. |
| PS5.1 package | `PASS` | `D220-PACKAGE-BUILD-PS51=PASS`; final PS5 candidate SHA `F08123CBAD384B4565F405BEA0903C339AB1D6894BE567BC16140A903E0F7137`. |
| PS7 package | `PASS` | `D220-PACKAGE-BUILD-PS7=PASS`; final manifest is bound to the PS7 candidate. |
| Package identity | `PASS` | `D220-PACKAGE-IDENTITY-PROBE=PASS`; root/dist SHA `C2BDAF2B2B452A97111EA9B01F0817EDBFA8091CA04BCECE58E73DD50E0FC389`, 38,565,121 bytes. |

## Public-source applicability

PyQt6 presentation code applies. Qt's public [Style Sheets
Reference](https://doc.qt.io/qt-6/stylesheet-reference.html) is the
applicable first-party source for selector/property ownership. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, and manufacturer requirements are not applicable.

## Unrun checks and reason

- GUI/QApplication and EXE startup, native QSS painting, screenshot,
  accessibility, font fallback, DPI, clean-machine, cross-machine,
  signing, installer/update, legal, support, permission/disk-pressure,
  hard-power, and release-owner checks were not run under the active no-launch
  or external-authorization policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not created
  or run under the project policy.
- `scripts/verify_release_handoff.ps1` remains expected `no-go` with the
  existing 10 open gates and three historical artifact-binding failures.

## Known risks and limits

- The checkout has no Git baseline; delegated architecture and independent
  windows did not conclude.
- Static token/selector evidence cannot prove native Qt metrics, painting,
  accessibility, or user-perceived beauty without an authorized visual run.
- Separate PyInstaller invocations can differ in bytes; the current PS7
  manifest is bound to its final candidate.

## Acceptance and evidence IDs

- Acceptance: `S271`
- Evidence: `D220-MAIN-SHELL-QSS-PROBE=PASS selectors=4 combos=12 gradients=0`,
  `D220-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D220-INDEPENDENT-REVIEW=NO_CONCLUSION`,
  `D220-SIMPLIFICATION-ASSESSMENT=PASS`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `C2BDAF2B2B452A97111EA9B01F0817EDBFA8091CA04BCECE58E73DD50E0FC389` /
  `38,565,121` bytes
- Source revision: `tree-sha256:d0673daad316cf9f876c9d79422a5cb6217043ffb9079025244c77f416780ad0`
- Root/dist identity: both paths match the final PS7 candidate.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue remaining visual-state, runtime, and release-gate audit;
  native visual acceptance still needs an authorized run.

## Disposition

`accepted-with-limits`: main-shell visual noise is reduced through the
centralized token-bound QSS boundary; native rendering and enterprise release
gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
