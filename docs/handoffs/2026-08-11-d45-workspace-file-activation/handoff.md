# Handoff: 2026-08-11-d45-workspace-file-activation

| Field | Value |
|---|---|
| ID | `2026-08-11-d45-workspace-file-activation` |
| Delivery / slice | `D45 / ARCH-35 / UI-31 Workspace file activation consistency` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T04:30:00+08:00` |

## User outcome

Workspace files now have an intuitive activation contract: single click,
double-click, and Enter/Return open a file; double-click and Enter/Return
enter a folder. A path already open in a tab is selected instead of being
opened again. The existing standalone Open dialog and asynchronous document
boundary remain unchanged.

## Scope and boundaries

### In scope

- WorkspacePanel double-click routing through the existing semantic intent
  helper.
- MainWindow busy guard, workspace containment recheck, and existing-tab reuse
  before the existing async document-open dispatch.
- English/Simplified Chinese workspace status guidance.
- Static architecture, review, handoff, package, and release evidence.

### Out of scope

- New services, coordinators, filesystem policy, TaskRunner changes, session or
  recovery policy, native dialog behavior, Qt startup, screenshots, runtime
  interaction, clean-machine, cross-machine, signing, installer, updater, and
  release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, boundary review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | File-opening reliability and modern workspace interaction |
| Developer | Parent architect | Smallest source change in panel/MainWindow/i18n |
| QA | Parent architect | Read-only source/static verification and unrun evidence |

## Decisions and constraints

- Keep the existing WorkspacePanel → WorkspaceSurface → MainWindow semantic
  route; do not introduce a new activation coordinator.
- Preserve single-click file opening and Enter/Return activation while making
  double-click intuitive for both files and folders.
- Apply busy and existing-tab guards before dispatch, then retain the existing
  workspace containment and asynchronous DocumentService policy.
- Avicenna the 2nd / Luna max architecture window returned no conclusion after
  three bounded waits; no architecture PASS is claimed. Feynman the 2nd /
  Luna max independent review also returned no conclusion after two bounded
  waits; no independent PASS is claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — double-click file/folder
  activation through the existing semantic route.
- `src/quillforge/presentation/main_window.py` — busy guard and existing-tab
  reuse before workspace file opening.
- `src/quillforge/presentation/i18n.py` — localized gesture guidance.
- `docs/adr/0070-workspace-file-activation-consistency.md` — decision,
  invariants, applicability, and simplification.
- `docs/agent-team/reviews/D45-workspace-file-activation-parent-review.md` and
  `docs/agent-team/reviews/D45-workspace-file-activation-independent-review.md`
  — review evidence and limits.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D45 workspace activation source probe | PASS | Click/double-click/keyboard, busy, tab-reuse, and locale branches. |
| `uv run python -m compileall -q src` | PASS | Authorized static compilation only. |
| `uv run ruff check src` | PASS | No diagnostics. |
| `uv run ruff format --check src` | PASS | Source is formatted. |
| `scripts\verify_handoff.ps1` | PASS | Indexed handoff and required sections agree. |
| `scripts\check.ps1` | PASS | Repository static, JSON, lint, format, and handoff checks pass. |
| `pwsh -NoProfile -File scripts\package.ps1` | PASS | Root/dist identity matches the artifact recorded below. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | 10 open gates and three known mechanical report-binding failures remain. |

## Unrun checks and reason

- Native Qt signal/event ordering, double-click timing, runtime file opening,
  focus/accessibility, DPI/font metrics, clean-machine/cross-machine behavior,
  and release-owner decisions — outside the active no-launch policy or require
  external authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native Qt click/double-click ordering or
  the exact timing of a worker completion racing with a second mouse event.
- Existing-tab reuse depends on the current path identity contract and does
  not change standalone Open-dialog duplicate-path policy.
- Runtime visual/accessibility, DPI/font metrics, clean-machine,
  cross-machine, and release-owner gates remain open.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Acceptance and evidence IDs

- Acceptance: `D45-AC01`, `S74`.
- Evidence: ADR-0070, parent/independent reviews, D45 source probe, static
  checks, handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue with the next distinct visual or coordinator slice only
  after the independent review and traceability/package checks are recorded;
  pursue D7/D8 runtime and release gates only with explicit authorization.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identity matches.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the bounded file-activation correction is integrated;
native interaction and release-owner gates remain conditions for later work.
