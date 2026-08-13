# Handoff: 2026-08-11-d51-settings-save

| Field | Value |
|---|---|
| ID | `2026-08-11-d51-settings-save` |
| Delivery / slice | `D51 / ARCH-41 Settings-save callback boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T10:00:00+08:00` |

## User outcome

Settings saves now have explicit operation identity and stale/invalid/failure
handling. Existing theme, language, font, editor, animation, notification,
and close behavior remains in the MainWindow presentation policy.

## Scope and boundaries

### In scope

- Qt-free `SettingsSaveTracker` for one settings-save operation ID and
  callback classification.
- MainWindow integration without moving SettingsService, TaskRunner, dialog,
  settings application, theme/locale/font/editor projection, animation,
  notifications, or close policy.
- Static architecture, simplification, review, handoff, package, and release
  evidence.

### Out of scope

- Settings schema/normalization, SettingsStore filesystem behavior, dialog
  visual changes, theme/font rendering, locale catalog changes, runtime
  interaction, screenshots, clean-machine/cross-machine evidence, signing,
  installer, updater, legal, support, and release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Contract, integration, verification, and handoff |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and release status |
| Product | User / product owner | Reliable settings persistence and visible failure behavior |
| Developer | Parent architect | Smallest source change in tracker/MainWindow integration |
| QA | Parent architect | Read-only source/static verification and unrun evidence |
| Independent review | Tesla the 2nd / Luna max | Read-only review; no conclusion returned |

## Decisions and constraints

- The tracker owns only operation identity and callback classification.
  MainWindow remains the sole owner of settings application and UI policy.
- Stale callbacks cannot unlock or mutate a current settings save; invalid and
  failed matching callbacks preserve existing error projection.
- Hooke the 2nd / Luna max was consulted as the required architecture role;
  two bounded windows returned no conclusion, so no architecture PASS is
  claimed. Tesla the 2nd / Luna max independent review also returned no
  conclusion after two bounded waits.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch is not authorized; source, compilation, packaging, and
  release-handoff evidence are the permitted validation boundary.

## Changed files and modules

- `src/quillforge/presentation/settings_save_tracker.py` — Qt-free settings
  callback lifecycle state.
- `src/quillforge/presentation/main_window.py` — delegates save identity while
  retaining settings application policy.
- `docs/adr/0076-settings-save-callback-boundary.md` — architecture decision.
- `docs/agent-team/reviews/D51-settings-save-parent-review.md` and
  `D51-settings-save-independent-review.md` — review evidence.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D51 settings behavior probe | `PASS` | Duplicate, stale, invalid, failure, and valid paths. |
| D51 source/integration probe | `PASS` | Qt-free boundary, policy projection, runner dispatch, and close guard. |
| `uv run python -m compileall -q src` | `PASS` | Authorized static compilation only. |
| `uv run ruff check src` | `PASS` | No diagnostics after source integration. |
| `uv run ruff format --check src` | `PASS` | Source is formatted. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_handoff.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\check.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing open gates/report-binding failures remain; human handoff identity is checked. |

## Unrun checks and reason

- Native settings callback timing, actual settings-file persistence, runtime
  theme/font/locale rendering, screenshots, accessibility, DPI, clean-machine,
  cross-machine, permission/disk pressure, signing, installer, updater, legal,
  support, and release-owner evidence — outside the current no-launch or
  external-authorization boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native settings dialog behavior or
  actual settings-file durability.
- Independent review has no conclusion; the parent record does not upgrade it.
- The portable candidate is unsigned and not an installer; release remains
  `NO-GO` until external gates are closed.

## Acceptance and evidence IDs

- Acceptance: `D51-AC01`, `S80`.
- Evidence: ADR-0076, parent/independent reviews, D51 probes, static checks,
  handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next coordinator/contract audit or obtain authorized
  runtime settings and release evidence before strengthening this claim.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `808F2EEE1AF0AB35B9B8C128D0E7F903472F9684776FE05141F0D1E2725A8E7C` /
  `38,427,064` bytes; root/dist identity matches.
- Source revision:
  `tree-sha256:3d32b03c82cf931ab4fdbb62d7341d82bd37c16275bcde4315114383c560deb2`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: settings-save callback identity is integrated and
statically verified; independent review, native runtime, and release-owner
gates remain conditions for later work.
