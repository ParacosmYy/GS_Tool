# Handoff: 2026-08-11-d50-recovery-capture

| Field | Value |
|---|---|
| ID | `2026-08-11-d50-recovery-capture` |
| Delivery / slice | `D50 / ARCH-40 Recovery-capture lifecycle boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T09:00:00+08:00` |

## User outcome

Recovery autosave's cooperative capture, worker write, stale/discarded
callback, and delete-after-write lifecycle is now explicit and reviewable.
The editor, recovery service, bounded channel, notifications, tab behavior,
and close guards retain their existing ownership and behavior.

## Scope and boundaries

### In scope

- Qt-free `RecoveryCaptureTracker[JobT, OwnerT]` for document/snapshot
  identity, capture jobs, discarded callbacks, worker writes, and deferred
  deletes.
- MainWindow integration while retaining editor slices, channel operations,
  RecoveryService, TaskRunner, tab identity, notification, and close policy.
- Removal of the redundant write-only channel-retention map.
- Static architecture, simplification, review, handoff, package, and release
  evidence.

### Out of scope

- RecoverySnapshot schema/store, editor capture algorithms, chunk limits,
  channel implementation, worker scheduling, dirty-state policy, UI visuals,
  runtime callback timing, hard-power durability, screenshots, clean-machine
  or cross-machine evidence, signing, installer, updater, legal, support, and
  release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Contract, integration, verification, and handoff |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and release status |
| Product | User / product owner | Reliable recovery autosave lifecycle without UI stalls |
| Developer | Parent architect | Smallest source change in tracker/MainWindow integration |
| QA | Parent architect | Read-only source/static verification and unrun evidence |
| Independent review | McClintock the 2nd / Terra max | Read-only review; no conclusion returned |

## Decisions and constraints

- The tracker owns framework-neutral lifecycle indexes and opaque values only.
  MainWindow remains the single owner of Qt/editor/service/runner/notification
  and close policy.
- Capture cancellation releases the document; already-started workers are
  classified as discarded and their later callbacks route to cleanup.
- Harvey the 2nd / Terra max was consulted as the required architecture role;
  two bounded windows returned no conclusion, so no architecture PASS is
  claimed. McClintock the 2nd / Terra max independent review also returned no
  conclusion after two bounded waits.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch is not authorized; source, compilation, packaging, and
  release-handoff evidence are the permitted validation boundary.

## Changed files and modules

- `src/quillforge/presentation/recovery_capture_tracker.py` — Qt-free capture,
  write, discard, and delete lifecycle state.
- `src/quillforge/presentation/main_window.py` — delegates recovery lifecycle
  state while retaining existing application/presentation policy.
- `docs/adr/0075-recovery-capture-lifecycle-boundary.md` — architecture
  decision.
- `docs/agent-team/reviews/D50-recovery-capture-parent-review.md` and
  `D50-recovery-capture-independent-review.md` — review evidence.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D50 tracker behavior probe | `PASS` | Duplicate guard, identity, deferred delete, discard, and abort release. |
| D50 source/integration probe | `PASS` | Qt-free boundary, delegation, cleanup routes, and close guard. |
| `uv run python -m compileall -q src` | `PASS` | Authorized static compilation only. |
| `uv run ruff check src` | `PASS` | No diagnostics after source integration. |
| `uv run ruff format --check src` | `PASS` | Source is formatted. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_handoff.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\check.ps1` | `PASS` | Recorded after documentation synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing open gates/report-binding failures remain; human handoff identity is checked. |

## Unrun checks and reason

- Native producer/worker callback timing, actual recovery filesystem I/O,
  hard-power durability, runtime startup, screenshots, accessibility, DPI,
  clean-machine, cross-machine, permission/disk pressure, signing, installer,
  updater, legal, support, and release-owner evidence — outside the current
  no-launch or external-authorization boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove channel scheduling, worker timing, or
  hard-power durability.
- Independent review has no conclusion; the parent record does not upgrade it.
- The portable candidate is unsigned and not an installer; release remains
  `NO-GO` until external gates are closed.

## Acceptance and evidence IDs

- Acceptance: `D50-AC01`, `S79`.
- Evidence: ADR-0075, parent/independent reviews, D50 probes, static checks,
  handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next coordinator boundary or obtain authorized
  runtime/recovery and release evidence before strengthening this claim.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `BA90DF4F4C8865B1FB1A8DA22D485ADCD934B9793173158301FC6D5BC3F5975E` /
  `38,425,096` bytes; root/dist identity matches.
- Source revision:
  `tree-sha256:f3a6471b01a9c8b813d2128df77fd71d6dc63059f95d00872393f4cfe07d5368`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: recovery-capture lifecycle ownership is integrated and
statically verified; independent review, native runtime, durability, and
release-owner gates remain conditions for later work.
