# Handoff: 2026-08-11-d52-replace-all-state

| Field | Value |
|---|---|
| ID | `2026-08-11-d52-replace-all-state` |
| Delivery / slice | `D52 / ARCH-42 Replace All lifecycle boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T11:00:00+08:00` |

## User outcome

Replace All now keeps a stable lifecycle identity across cooperative UI
slices. If the user cancels and immediately starts another operation, a stale
queued callback from the old operation cannot advance the new editor session.
Existing replacement, cancellation, rollback, progress, status, and dirty
marker behavior remains owned by the existing presentation policy.

## Scope and boundaries

### In scope

- Qt-free `ReplaceAllTracker` and typed `ReplaceAllJob` for active identity,
  expected content version, and progress-report state.
- Job-bound QTimer callbacks and stale callback guards in MainWindow.
- Static architecture, simplification, review, handoff, package, and release
  evidence.

### Out of scope

- ReplaceAllSession/editor algorithms, QScintilla behavior, timer policy,
  rollback implementation, UI copy/theme changes, runtime interaction,
  screenshots, clean-machine/cross-machine evidence, signing, installer,
  updater, legal, support, and release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Contract, integration, verification, and handoff |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and release status |
| Product | User / product owner | Reliable Replace All cancellation and replacement behavior |
| Developer | Parent architect | Smallest source change in tracker/MainWindow integration |
| QA | Parent architect | Read-only source/static verification and unrun evidence |
| Independent review | Zeno the 2nd / Terra max | Read-only review; no conclusion returned |

## Decisions and constraints

- The tracker owns only the current job record and identity/content-version
  lifecycle. MainWindow remains the sole owner of editor/session/timer,
  operation, rollback, status, notification, and close policy.
- Every queued callback captures its job identity. A stale callback returns
  before session stepping, content-version update, or cleanup.
- Galileo the 2nd / Terra max was consulted as the required architecture
  role; three bounded windows returned no conclusion, so no architecture PASS
  is claimed. Zeno the 2nd / Terra max independent review also returned no
  conclusion after two bounded waits.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch is not authorized; source, compilation, packaging, and
  release-handoff evidence are the permitted validation boundary.

## Changed files and modules

- `src/quillforge/presentation/replace_all_tracker.py` — Qt-free job identity
  and content-version lifecycle state.
- `src/quillforge/presentation/main_window.py` — delegates Replace All state
  and binds QTimer callbacks to job identity while retaining policy.
- `docs/adr/0077-replace-all-lifecycle-boundary.md` — architecture decision.
- `docs/agent-team/reviews/D52-replace-all-state-parent-review.md` and
  `D52-replace-all-state-independent-review.md` — review evidence.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D52 Replace All identity/stale probe | `PASS` | Duplicate, current/stale, version, finish, and invalid-ID paths. |
| D52 source/callback-boundary probe | `PASS` | Qt-free tracker, callback identity, stale guard, and policy seam. |
| `uv run python -m compileall -q src` | `PASS` | Authorized static compilation only. |
| `uv run ruff check src` | `PASS` | No diagnostics after PEP 695 correction. |
| `uv run ruff format --check src` | `PASS` | Source is formatted. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_handoff.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\check.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing open gates/report-binding failures remain. |

## Unrun checks and reason

- Native event-loop callback interleavings, editor rollback durability,
  runtime rendering, screenshots, accessibility, DPI, clean-machine,
  cross-machine, permission/disk pressure, signing, installer, updater,
  legal, support, and release-owner evidence — outside the current no-launch
  or external-authorization boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native QTimer ordering or actual editor
  session behavior under every runtime interleaving.
- Independent review has no conclusion; the parent record does not upgrade it.
- The portable candidate is unsigned and not an installer; release remains
  `NO-GO` until external gates are closed.

## Acceptance and evidence IDs

- Acceptance: `D52-AC01`, `S81`.
- Evidence: ADR-0077, parent/independent reviews, D52 probes, static checks,
  handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct coordinator/contract audit or obtain
  authorized runtime Replace All and release evidence before strengthening
  this claim.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `212FEA8A77214A5C4EC3FF0776B167177900E1661D858A65A388C4D7A6057989` / `38,430,550` bytes; root/dist identity matches.
- Source revision: `tree-sha256:89175f8be1f0864f8b77dc4f9c5418f1f622137189bea46a2433536d93fae5ae`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: Replace All callback identity is integrated and
statically verified; independent review, native runtime, and release-owner
gates remain conditions for later work.
