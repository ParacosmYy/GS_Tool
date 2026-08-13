# Handoff: 2026-08-11-d194-native-control-affordance

| Field | Value |
|---|---|
| ID | `2026-08-11-d194-native-control-affordance` |
| Delivery / slice | `D194 / UI-104 / ARCH-180 Native control affordance cohesion` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

ComboBox and SpinBox arrows now share the selected QuillForge theme's geometry,
foreground, open, and disabled states. A checked checkbox retains a visible
accent focus boundary. Existing settings behavior, signals, and appearance
contracts remain unchanged.

## Scope and boundaries

### In scope

- Central QSS subcontrol projection in `presentation/theme.py`.
- Static token/selector/contrast evidence and the portable package refresh.

### Out of scope

- Widget construction, signals, values, settings persistence, locale, motion,
  application services, and domain behavior.
- Native QSS rendering, accessibility-tree output, DPI, screenshot comparison,
  GUI/EXE startup, clean-machine, cross-machine, legal, signing, installer,
  updater, support, and release-owner acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Consistent modern/cute themed controls |
| Developer | `parent` | Central QSS projection |
| QA | `parent` | Read-only source, static, handoff, and package verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — explicit ComboBox/SpinBox arrows
  and checked-checkbox focus selector.
- `tasks/plan.md`, `tasks/todo.md`, and synchronized architecture/release
  records.

## Decisions and constraints

- Shared checkout writer: `parent`, one bounded presentation slice.
- Runtime launch policy: not allowed; no GUI, EXE, screenshot, native popup,
  or test-only asset was run/created.
- Architecture window: `Huygens the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child PASS is claimed.
- Independent review: `Bernoulli the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Python/PyQt6 presentation only; embedded C/C++, MCU, RTOS, and manufacturer
  requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Generated-QSS selector probe | `PASS` | `D194-CONTROL-AFFORDANCE-QSS-PROBE=PASS`; 12 theme/accent combinations and 8 selectors. |
| Token contrast probe | `PASS` | `D194-CONTROL-AFFORDANCE-CONTRAST-PROBE=PASS`; targeted small-affordance pairs met the 3:1 threshold. |
| `uv run python -m compileall -q src` | `PASS` | No GUI startup. |
| `uv run ruff check src scripts` | `PASS` | No lint diagnostics. |
| `uv run ruff format --check src scripts` | `PASS` | All files already formatted. |
| `scripts/check.ps1` | `PASS` | Presentation and handoff checks passed. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff checks passed before the release dossier refresh. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/package.ps1` | `PASS` | Portable candidate rebuilt; EXE not launched. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | 10 external/open gates and three stale artifact-bound consistency failures remain. |

## Unrun checks and reason

- Native QSS parsing/painting, accessibility, DPI, screenshot comparison, GUI/
  EXE startup, focus traversal, and native popup behavior — prohibited by the
  current no-launch policy.
- Clean-machine, cross-machine, signing, installer, updater, legal, support,
  permission/disk-pressure, and release-owner checks — external authorization
  or evidence remains pending.

## Known risks and limits

- Qt subcontrol specificity and native metrics can differ by platform/style;
  runtime rendering remains an explicit acceptance limit.
- The first package invocation through Windows PowerShell 5.1 reached EXE
  creation but failed at the existing `Path.GetRelativePath` manifest step;
  the supported PowerShell 7 invocation completed and produced the identity
  below. PowerShell 5.1 compatibility remains a follow-up release-tool gap.
- Existing release status remains `no-go`.

## Acceptance and evidence IDs

- Acceptance: `S246`
- Evidence: `D194-CONTROL-AFFORDANCE-QSS-PROBE=PASS`,
  `D194-CONTROL-AFFORDANCE-CONTRAST-PROBE=PASS`,
  `D194-COMPILE-RUFF-FORMAT=PASS`, `D194-PRESENTATION-AUDIT=PASS`,
  `D194-SIMPLIFICATION-ASSESSMENT=PASS`, `D194-PACKAGE-IDENTITY-PROBE=PASS`,
  `D194-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D194-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: QA / Product
- Action: authorize native control rendering/accessibility review before
  treating this as runtime accepted; separately schedule PowerShell 5.1
  packaging compatibility if Windows PowerShell support is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `8E5924C83FBE368E3CB608DE0EB66DDE29F1269A8E4CC8A83BF3AA89E37A47E1` /
  `38,561,269` bytes
- Source revision: `tree-sha256:21ddb86b232ed0f4e3a1e91c860f0e9a373fbbcec2c7d5f3d143961eed1b9bf8`
- Packaging note: portable PyInstaller one-file candidate rebuilt with
  PowerShell 7; no installer or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the native-control QSS boundary and static/package
evidence are recorded; native rendering, runtime, PowerShell 5.1 packaging
compatibility, and enterprise release gates remain open.
