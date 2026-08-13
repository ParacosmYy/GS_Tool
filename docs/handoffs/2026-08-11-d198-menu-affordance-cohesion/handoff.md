# Handoff: 2026-08-11-d198-menu-affordance-cohesion

| Field | Value |
|---|---|
| ID | `2026-08-11-d198-menu-affordance-cohesion` |
| Delivery / slice | `D198 / UI-105 / ARCH-184 Menu affordance cohesion` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

The centralized menu style now gives checkable actions an authored indicator
surface and an explicit selected-and-checked hierarchy across every supported
theme/accent combination. Existing commands remain behaviorally unchanged.

## Scope and boundaries

### In scope

- `QMenu::item:checked` and `QMenu::item:selected:checked` QSS states.
- `QMenu::indicator` normal, hover, checked, checked-hover, and disabled
  states, all bound to existing `ThemeColors` tokens.
- Static source, contrast, contract, compile, format, package, and handoff
  evidence.

### Out of scope

- QAction behavior, command registration, locale, shortcuts, and callbacks.
- GUI/QApplication startup, screenshot/native menu rendering, accessibility,
  DPI, clean-machine, cross-machine, installer, updater, registry/file
  association, signing, legal, support, and release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Modern, readable, state-distinct menu styling |
| Developer | `parent` | Centralized menu QSS projection |
| QA | `parent` | Static selector/contrast, compile, package, and handoff verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized menu indicator and
  selected-checked state rules.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized architecture/release
  records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded presentation slice.
- Runtime launch policy: not allowed; no GUI, QApplication, EXE, screenshot,
  or test-only asset was run/created.
- Architecture window: `Hume the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Independent review: `Locke the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Generated QSS selector/contrast probe | `PASS` | `D198-MENU-AFFORDANCE-QSS-PROBE=PASS combinations=12 selectors=5`; `D198-CONTRAST-PROBE=PASS combinations=12`. |
| `uv run python -m compileall -q src scripts` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | `PASS` | Presentation ownership audit passed. |
| `scripts/package.ps1` under Windows PowerShell 5.1 | `PASS` | `D198-PACKAGE-BUILD-PS51=PASS`; EXE not launched. |
| `scripts/package.ps1` under PowerShell 7 | `PASS` | `D198-PACKAGE-BUILD-PS7=PASS`; EXE not launched. |

## Unrun checks and reason

- GUI/QApplication startup, native QSS/menu painting, screenshot, accessibility,
  DPI, packaged capture, performance reports, clean-machine, cross-machine,
  installer/updater, registry/file association, signing, legal, support,
  permission/disk-pressure, hard-power, and release-owner checks — prohibited
  by the active no-launch policy or require external authorization/evidence.

## Known risks and limits

- Native Qt stylesheet specificity, indicator geometry, menu metrics, font
  fallback, and accessibility behavior still require authorized runtime review.
- The checkout has no Git baseline; architecture and independent review
  windows returned `NO_CONCLUSION`, so parent review is the only PASS claim.
- Artifact-bound runtime reports remain stale by design and keep the release
  decision at no-go.

## Public-source applicability

Qt's public Style Sheets Reference is the applicable first-party source for
the QSS subcontrols. This project is Python/PyQt6 presentation code; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `13B13914C1BBA34565B34F570EDC3040D2EF48DD2C862EA8852BBA24AD63ED74` /
  `38,561,389` bytes
- Source revision: `tree-sha256:b901d3a0a0a0eb0f0f4ceec73660f409724798217c65ed4c5826366949b921e6`
- Packaging note: portable PyInstaller one-file candidate rebuilt under both
  shells; no installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the menu affordance source and static/package identity
are recorded; native menu rendering, runtime evidence, and enterprise release
gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S250`
- Evidence: `D198-MENU-AFFORDANCE-QSS-PROBE=PASS`,
  `D198-CONTRAST-PROBE=PASS`, `D198-COMPILE-RUFF-FORMAT=PASS`,
  `D198-PRESENTATION-AUDIT=PASS`, `D198-PACKAGE-BUILD-PS51=PASS`,
  `D198-PACKAGE-BUILD-PS7=PASS`, `D198-PACKAGE-IDENTITY-PROBE=PASS`,
  `D198-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D198-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize native menu/runtime visual review and refresh
  artifact-bound reports before treating this slice as runtime validated.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
