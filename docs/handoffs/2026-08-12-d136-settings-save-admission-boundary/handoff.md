# Handoff: 2026-08-12-d136-settings-save-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d136-settings-save-admission-boundary |
| Delivery / slice | D136 / ARCH-114 settings-save admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T03:30:00+08:00 |

## User outcome

Settings saves now have one explicit typed admission boundary. Service
availability, in-flight protection, candidate editing, operation identity,
tracker binding, and asynchronous dispatch remain ordered while persistence,
theme/locale/font/motion application, result classification, and close policy
retain their existing owners.

## Scope and boundaries

### In scope

- `SettingsSaveAdmissionCoordinator` and `SettingsSaveAdmissionPorts`.
- `MainWindow` composition wiring and `_show_settings` delegation.
- Static behavior/contract/Qt-free probes, package, and traceability records.

### Out of scope

- No `SettingsSurface`, `SettingsService` implementation, normalization,
  `SettingsSaveCoordinator`, `SettingsSaveProjectionCoordinator`,
  QApplication/theme/locale/font/editor/motion projection, notification,
  close, or application-policy rewrite.
- No QApplication launch, native dialog, settings-file durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Confucius the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Descartes the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/settings_save_admission_coordinator.py` — new
  Qt-free admission contract and sequencing.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_show_settings` delegation only.
- `docs/adr/0176-settings-save-admission-boundary.md`
- `docs/agent-team/reviews/D136-settings-save-admission-parent-review.md`
- `docs/agent-team/reviews/D136-settings-save-admission-independent-review.md`

## Decisions and constraints

- Admission, completion, and projection remain separate contracts; result
  classification stays in `SettingsSaveCoordinator`.
- The settings service is composition-bound and captured before the modal
  edit, preserving the existing service ownership and worker operation shape.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D136-SETTINGS-ADMISSION-BEHAVIOR-PROBE=PASS`
- `D136-SETTINGS-ADMISSION-CONTRACT-PROBE=PASS`
- `D136-QT-FREE-SETTINGS-PROBE=PASS`
- `D136-COMPILEALL=PASS`
- `D136-RUFF=PASS`
- `D136-FORMAT=PASS`
- `D136-CHECK=PASS`
- `D136-VERIFY-HANDOFF=PASS`
- `D136-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication startup, native settings interaction, worker callback timing,
  settings-file durability, theme/font/locale/motion rendering,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission evidence cannot prove native SettingsDialog behavior or
  queued worker timing on every Windows environment.
- Confucius architecture and Descartes independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S180`, `D136-AC01`.
- Evidence: ADR-0176, settings-admission behavior/contract/Qt-free probes,
  parent and independent review records, simplification assessment, static
  checks, package identity, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or distinct visual
  gap slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `5C4FAF3F840404F51819D9C583C196A4CB7F2D73FCFB6ADD93E996EC718B4D0A`
- Size: `38523533` bytes
- Source revision: `tree-sha256:74375b459acb6d578866c67f620a557be03d9f12e83dcb04ccc55bb0f0669383`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: settings-save admission is centralized at a typed
presentation boundary while native/runtime/release evidence remains open.
