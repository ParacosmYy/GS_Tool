# Handoff: 2026-08-11-d41-workspace-entry-activation

| Field | Value |
|---|---|
| ID | `2026-08-11-d41-workspace-entry-activation` |
| Delivery / slice | `D41 / ARCH-31 / UI-27 Workspace entry activation` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T00:30:00+08:00` |

## User outcome

Workspace files retain the existing single-click open behavior and folders
retain double-click navigation. Keyboard activation now opens focused files or
enters focused folders through the same semantic presentation boundary, which
closes a visible file-opening accessibility and reliability gap.

## Scope and boundaries

### In scope

- `WorkspacePanel` item activation routing for file and directory entries.
- Existing mouse compatibility routes and inaccessible-entry inertness.
- Static source, architecture, handoff, package, and release evidence.

### Out of scope

- MainWindow containment, document loading, workspace enumeration, TaskRunner,
  notifications, or async/stale-operation policy.
- Qt startup, native event-order acceptance, screenshots, accessibility runtime,
  clean-machine, cross-machine, signing, installer, update, and release-owner
  approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | File-opening outcome and acceptance |
| Developer 1 | Parent architect | WorkspacePanel activation implementation |
| Developer 2 | Parent architect | Contract/docs/package synchronization |
| QA | Parent architect | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — add Enter/Return-only
  keyboard activation and centralize file/directory intent projection.
- `docs/adr/0066-workspace-entry-activation.md` — architecture decision,
  public-source applicability, simplification, and limits.
- `docs/agent-team/reviews/D41-workspace-entry-activation-parent-review.md` —
  parent review and independent-review status.
- `docs/agent-team/reviews/D41-workspace-entry-activation-independent-review.md`
  — initial finding, correction record, and post-correction independent PASS.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` —
  traceability and package evidence.

## Decisions and constraints

- Preserve single-click file opening and double-click directory navigation;
  add keyboard activation without changing application policy.
- Keep `WorkspacePanel` presentation-only and emit semantic callbacks with the
  visible item path; no filesystem call or document service enters the widget.
- The Terra architecture consultation (Dewey the 2nd / Terra max) returned no
  conclusion after bounded waits; no architecture PASS is claimed. Faraday the
  2nd / Luna max initially returned FAIL, the parent corrected the signal and
  nullable-item issues, and Turing the 2nd / Luna max independently reviewed
  the correction with PASS and explicit runtime limits.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D41 workspace-entry activation source probe | PASS | Enter/Return-only keyboard route, no live `itemActivated` connection, kind-aware helper, matching callbacks, null safety, unchanged mouse routes, and no service imports. |
| `uv run python -m compileall -q src/quillforge/presentation/workspace_panel.py` | PASS | Authorized static compilation only. |
| `uv run ruff check src/quillforge/presentation/workspace_panel.py` | PASS | No diagnostics. |
| `uv run ruff format --check src/quillforge/presentation/workspace_panel.py` | PASS | File already formatted. |
| `scripts\verify_handoff.ps1` | PASS | Indexed handoff and Markdown status/required headings agree. |
| `scripts\check.ps1` | PASS | Repository static and traceability checks pass. |
| `scripts\package.ps1` | PASS | Root/dist portable candidates share the recorded D41 identity. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Existing runtime-report and external release gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, native event ordering, mouse/keyboard activation,
  focus traversal, screenshots, screen-reader output, font metrics, DPI, and
  cross-machine appearance — blocked by the active no-launch policy and lack
  of authorized runtime acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- The rejected style-dependent `itemActivated` route is not connected. Native
  Qt event ordering, focus state, and actual key/mouse delivery remain
  unverified because runtime launch is outside the authorized boundary.
- Static source evidence cannot prove actual keyboard focus traversal or file
  opening in a native event loop.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Acceptance and evidence IDs

- Acceptance: `D41-AC01`, `S70`.
- Evidence: `docs/adr/0066-workspace-entry-activation.md`,
  `docs/agent-team/reviews/D41-workspace-entry-activation-parent-review.md`,
  `docs/agent-team/reviews/D41-workspace-entry-activation-independent-review.md`,
  `D41-workspace-entry-activation-probe=PASS`, Turing the 2nd post-correction
  independent PASS with limits, static checks, and `dist/QuillForge.release.json`.

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

`accepted-with-limits`: the D41 activation-route source slice is integrated
with explicit architecture, review status, simplification, and static
verification evidence. Native event ordering and release-owner gates remain
conditions for later work.
