# Handoff: 2026-08-11-d47-session-restore-state

| Field | Value |
|---|---|
| ID | `2026-08-11-d47-session-restore-state` |
| Delivery / slice | `D47 / ARCH-37 / UI-33 Session-restore state boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T06:30:00+08:00` |

## User outcome

The startup session restore flow retains its existing behavior while its
framework-neutral state is explicit and extensible: ordered documents, the
workspace barrier, active path, recovery deferrals, and the async document/open
binding are isolated from the Qt coordinator. This supports continued
enterprise architecture migration without moving application policy into a
generic state object.

## Scope and boundaries

### In scope

- `SessionRestoreTracker` Qt-free value-state boundary.
- MainWindow integration preserving serial restore, recovery-first ordering,
  workspace gating, tab reuse, caret/active-tab projection, notifications, and
  save/close policy.
- Static source, architecture, review, handoff, package, and release evidence.

### Out of scope

- Session schema, persistence format, recovery service, document/workspace
  service, TaskRunner, tab widgets, notifications, or close-policy rewrites.
- Qt startup, native callback timing, runtime file/recovery I/O, screenshots,
  clean-machine/cross-machine evidence, signing, installer, updater, legal,
  support approval, and release-owner gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, boundary decision, verification, and handoff |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and release status |
| Product | User / product owner | Continuity and maintainable modernization outcome |
| Developer | Parent architect | Smallest source change in tracker/MainWindow |
| QA | Parent architect | Read-only source/static verification and unrun evidence |
| Independent review | Heisenberg the 2nd / Luna max | Read-only review; no conclusion returned |

## Decisions and constraints

- The tracker owns typed restore values and transitions only. MainWindow keeps
  services, TaskRunner, generic operation state, tab projection, notifications,
  startup/close guards, and result policy.
- `has_remaining_documents` is non-consuming; `next_document()` consumes one
  item in order. A pending document is recorded before async dispatch and
  bound to its operation ID.
- Huygens the 2nd / Luna max was consulted for the restore chain and Dirac the
  2nd / Terra max was used for escalation of the cross-module review; both
  bounded architecture windows returned no conclusion. No architecture PASS
  is claimed.
- Heisenberg the 2nd / Luna max returned no conclusion after two bounded
  independent-review waits; no independent PASS is claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch is not authorized; source, compilation, packaging, and
  release-handoff evidence are the permitted validation boundary.

## Changed files and modules

- `src/quillforge/presentation/session_restore_tracker.py` — Qt-free ordered
  restore state and callback binding.
- `src/quillforge/presentation/main_window.py` — coordinator integration with
  legacy restore fields removed.
- `docs/adr/0072-session-restore-state-boundary.md` — architecture decision.
- `docs/agent-team/reviews/D47-session-restore-state-parent-review.md` and
  `D47-session-restore-state-independent-review.md` — review evidence.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Session-restore boundary source probe | `PASS` | Qt-free tracker, legacy-field removal, queue order, callback binding. |
| `uv run python -m compileall -q src` | `PASS` | Authorized static compilation only. |
| `uv run ruff check src` | `PASS` | No diagnostics. |
| `uv run ruff format --check src` | `PASS` | Source is formatted. |
| `pwsh -NoProfile -File scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff and required sections agree. |
| `scripts\check.ps1` | `PASS` | Repository static, JSON, lint, format, and handoff checks pass. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | 10 open gates and three known report-binding failures remain; human handoff identity matches. |

## Unrun checks and reason

- Native Qt callback ordering, recovery/file I/O, thread timing, actual
  startup, screenshots, accessibility, DPI, clean-machine, cross-machine,
  permission/disk pressure, hard-power, signing, installer, updater, legal,
  support, and release-owner evidence — outside the current no-launch or
  external-authorization boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native Qt queued-callback interleavings
  or service/file-system behavior.
- Independent review has no conclusion; the parent review records evidence and
  limits without upgrading that status.
- The portable candidate is unsigned and not an installer; release remains
  `NO-GO` until the external gates are closed.

## Acceptance and evidence IDs

- Acceptance: `D47-AC01`, `S76`.
- Evidence: ADR-0072, parent/independent reviews, D47 source probe, static
  checks, handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: obtain an independent review or authorized runtime evidence before
  strengthening the restore claim; continue the next bounded coordinator slice
  only after preserving this boundary.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A48C8F0C2D66B5FAEED15EBAC1D4ADA72C0F2C394F777E9B8D29CF10613DA772` /
  `38,417,701` bytes; root/dist identity matches.
- Source revision:
  `tree-sha256:b117561a2a38ea8e62d42d6dc77ef11a1158e3e54511b278ccfa10a0f14b8dfd`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the restore value-state boundary is integrated and
statically verified; independent review, native runtime, and release-owner
gates remain conditions for later work.
