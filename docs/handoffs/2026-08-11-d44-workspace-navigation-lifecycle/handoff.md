# Handoff: 2026-08-11-d44-workspace-navigation-lifecycle

| Field | Value |
|---|---|
| ID | `2026-08-11-d44-workspace-navigation-lifecycle` |
| Delivery / slice | `D44 / ARCH-34 / UI-30 Workspace-navigation lifecycle boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T03:30:00+08:00` |

## User outcome

Workspace folder open/list callbacks now share one explicit lifecycle boundary
for operation identity and generation invalidation. Existing folder
navigation, cancellation feedback, file opening, session restore, and stale
callback behavior remain owned by the same MainWindow policy paths.

## Scope and boundaries

### In scope

- Qt-free `WorkspaceOperationTracker` for workspace navigation lifecycle
  identity and generation classification.
- MainWindow delegation for workspace open/list start, cancellation,
  completion, failure, and session-restore callback paths.
- Static architecture, source, handoff, package, and release evidence.

### Out of scope

- Generic busy/status operation policy, TaskRunner, WorkspaceService,
  WorkspaceSurface, containment, notifications, session-restore policy, or
  result validation.
- Qt startup, native queued delivery, thread timing, screenshots, runtime
  navigation, clean-machine, cross-machine, signing, installer, update, and
  release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | Workspace navigation reliability and architecture outcome |
| Developer 1 | Parent architect | Tracker implementation and MainWindow integration |
| Developer 2 | Parent architect | Contract/docs/package synchronization |
| QA | Parent architect | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/workspace_operation_tracker.py` —
  framework-neutral navigation lifecycle boundary.
- `src/quillforge/presentation/main_window.py` — delegate existing workspace
  lifecycle paths while retaining policy.
- `docs/adr/0069-workspace-navigation-lifecycle-boundary.md` — architecture
  decision, invariants, applicability, simplification, and limits.
- `docs/agent-team/reviews/D44-workspace-navigation-lifecycle-parent-review.md`
  — parent architecture/review record.
- `docs/agent-team/reviews/D44-workspace-navigation-lifecycle-independent-review.md`
  — explicit no-conclusion independent-review record.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` —
  traceability and package evidence.

## Decisions and constraints

- Keep generic busy/status ownership in `OperationTracker`/MainWindow; the new
  tracker owns only workspace identity and generation.
- Invalidate workspace lifecycle state before bridging to generic operation
  cancellation; stale worker callbacks cannot clear current state.
- Pauli the 2nd / Terra max architecture window returned no conclusion after
  two bounded waits; no architecture PASS is claimed. Pascal the 2nd / Luna
  max independent review window also returned no conclusion; no child PASS is
  claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D44 workspace-lifecycle source/boundary probe | PASS | Tracker transitions, no old MainWindow fields, generic cancellation bridge, retained policy owners. |
| `uv run python -m compileall -q src/quillforge/presentation/main_window.py src/quillforge/presentation/workspace_operation_tracker.py` | PASS | Authorized static compilation only. |
| `uv run ruff check src/quillforge/presentation/main_window.py src/quillforge/presentation/workspace_operation_tracker.py` | PASS | No diagnostics. |
| `uv run ruff format --check src/quillforge/presentation/main_window.py src/quillforge/presentation/workspace_operation_tracker.py` | PASS | Both files already formatted. |
| `scripts\verify_handoff.ps1` | PASS | Indexed handoff and Markdown status/required headings agree. |
| `scripts\check.ps1` | PASS | Repository static and traceability checks pass. |
| `scripts\package.ps1` | PASS | Root/dist portable candidates share the recorded D44 identity. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime-report consistency and external release gates remain open. |

## Unrun checks and reason

- Native queued delivery, real thread timing, workspace cancellation callback
  ordering, close-event interleavings, Qt startup, runtime navigation,
  screenshots, clean-machine, and cross-machine behavior — outside the active
  no-launch policy and without authorized runtime acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static lifecycle evidence cannot prove actual QThreadPool/queued-signal
  ordering or a close event racing with workspace completion.
- The tracker intentionally does not own TaskRunner or session policy, so later
  coordinator work must preserve those boundaries.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Acceptance and evidence IDs

- Acceptance: `D44-AC01`, `S73`.
- Evidence: `docs/adr/0069-workspace-navigation-lifecycle-boundary.md`,
  `docs/agent-team/reviews/D44-workspace-navigation-lifecycle-parent-review.md`,
  `docs/agent-team/reviews/D44-workspace-navigation-lifecycle-independent-review.md`,
  `D44-workspace-lifecycle-probe=PASS`, static checks, and
  `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: Architect.
- Action: continue with D7/D8 runtime, clean-machine, and release-owner gates
  when separately authorized.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identity matches.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the D44 navigation lifecycle boundary is integrated
with explicit architecture, review status, simplification, and static
verification evidence. Native callback ordering and release-owner gates remain
conditions for later work.
