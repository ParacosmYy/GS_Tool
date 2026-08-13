# Handoff: 2026-08-09-ui-06-status-coverage

| Field | Value |
|---|---|
| ID | `2026-08-09-ui-06-status-coverage` |
| Delivery / slice | `D9 / UI-06 TaskRunner status coverage` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T18:20:00+08:00` |

## User outcome

The QuillForge shell no longer presents READY while any shared `TaskRunner`
operation or queued completion callback remains retained. After the lifecycle
drains, the existing dirty-document ATTENTION state or clean-idle READY state
is restored without changing cancellation or close behavior.

## Scope and boundaries

### In scope

- One `TaskRunner.pending_changed` lifecycle signal.
- MainWindow status priority for all retained background operations.
- ADR, acceptance, delivery-register, review, and handoff traceability.

### Out of scope

- New application operations, cancellation semantics, or close-guard policy.
- QuillForge startup, Qt-window inspection, screenshots, DPI, accessibility,
  clean-machine, and cross-machine visual evidence.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrated the source, architecture decision, final review, and verification |
| Project Manager | Architect-led bounded record | Sequenced the follow-up from the UI-04 documented coverage gap |
| Product | Architect-led bounded record | Confirmed WORKING must cover retained background work and queued delivery |
| Developer 1 | Architect-led bounded record | Reviewed TaskRunner ownership and close-guard compatibility |
| Developer 2 | Architect-led bounded record | Implemented the presentation signal and MainWindow projection |
| QA | Architect-led bounded record | Ran static/workflow checks and recorded prohibited runtime checks |

No child PASS or independent review is claimed; the parent owns integration and
the final evidence decision in this checkout.

## Changed files and modules

- `src/quillforge/presentation/task_runner.py` — emits pending-count changes at
  retain/release boundaries.
- `src/quillforge/presentation/main_window.py` — projects pending work before
  dirty-document attention or idle READY.
- `docs/adr/0029-task-runner-status-projection.md` — records the lifecycle and
  status-priority decision.
- `docs/ARCHITECTURE.md` — aligns the shared TaskRunner and status projection
  boundary.
- `docs/agent-team/acceptance.json` — adds D9-AC05/S31 evidence contract and
  updates D9-AC03 coverage.
- `docs/agent-team/delivery-register.json` and `docs/ROADMAP.md` — record UI-06.
- `scripts/check.ps1` — requires the new acceptance IDs.
- `docs/agent-team/reviews/D9-UI-06-parent-review.md` — parent audit.
- `docs/RELEASE_HANDOFF.md` — records that previous launch/measurement evidence
  no longer matches the freshly rebuilt candidate.
- `docs/handoffs/index.json` — indexes this handoff as latest.

## Decisions and constraints

- `TaskRunner` remains the single retained-work observation point; no call site
  duplicates background-operation counters.
- Pending work has priority over dirty-document attention, and the pending set
  still includes queued completion delivery for close-safety alignment.
- Shared checkout writer: Architect, limited to the files listed above.
- Runtime launch policy: software and Qt windows remain forbidden by the local
  project instruction; static build/package verification is allowed.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run ruff check src scripts` | `PASS` | Required after the source change. |
| `uv run ruff format --check src scripts` | `PASS` | Required formatting gate. |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff/index structure verified after ledger update. |
| `scripts/check.ps1` | `PASS` | Acceptance, boundaries, lock, formatting, lint, and compile checks. |
| `scripts/package.ps1` | `PASS` | Fresh PyInstaller one-file build; root and dist hashes match. |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, and screenshots — intentionally
  unrun because the current user/project instruction prohibits software launch.
- Clean-machine, interactive visual, DPI, accessibility, and cross-machine
  checks — require a permitted external environment and user-owned review.
- `scripts/verify_release_handoff.ps1` — intentionally not rerun after the
  rebuild because its current evidence set is artifact-bound and refreshing it
  would require prohibited startup/diagnostic execution.
- Unit tests and test-only assets — not created or run under project policy.

## Known risks and limits

- The source contract proves lifecycle projection structurally; runtime visual
  appearance and native accessibility remain unverified.
- Cooperative cancellation remains cooperative; the status rail does not claim
  immediate worker interruption or OS-termination durability.
- D9 and the broader project remain in progress while release and visual gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S31`, `D9-AC05` (supplements `S28` / `D9-AC03`).
- Evidence: `src/quillforge/presentation/task_runner.py`,
  `src/quillforge/presentation/main_window.py`,
  `docs/adr/0029-task-runner-status-projection.md`,
  `docs/agent-team/reviews/D9-UI-06-parent-review.md`,
  `scripts/check.ps1`.

## Next owner and next action

- Owner: Architect / Product.
- Action: perform the user-permitted runtime visual review when the no-launch
  instruction is explicitly reversed, then reassess D9 acceptance and release
  gates.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `B384A91D387CF89DCAA38B6CAC22C6CD223A420BC46B3ED373610D34A488AE3D` / `38,325,902` bytes for both root and dist copies.
- Packaging note: fresh portable one-file candidate; release manifest records
  the same artifact identity and NOTICE digest.

## Disposition

`accepted-with-limits`: the TaskRunner-backed source projection closes the
independent background-operation coverage gap. Runtime visual and external
release gates remain explicitly open.
