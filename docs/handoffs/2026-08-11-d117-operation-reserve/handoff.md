# Handoff: 2026-08-11-d117-operation-reserve

| Field | Value |
|---|---|
| ID | 2026-08-11-d117-operation-reserve |
| Delivery / slice | D117 / ARCH-91 operation-reserve facade simplification |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T06:20:00+08:00 |

## User outcome

QuillForge's operation-ID allocation now uses the canonical
`OperationTracker.reserve` contract directly, reducing MainWindow indirection
while preserving session-save, workspace, recovery, settings, and document
operation sequencing.

## Scope and boundaries

### In scope

- Initialize the existing OperationTracker before session-save composition.
- Inject its bound `reserve` port into SessionSaveCoordinator.
- Replace seven pure reserve facade calls and remove `_next_operation_id()`.
- Preserve `_begin_operation()` / `_complete_operation()` policy behavior.

### Out of scope

- No OperationTracker allocation semantics, SessionSaveCoordinator result
  classification, TaskRunner dispatch, stale guards, busy/status, close,
  persistence, notification, document, recovery, or workspace policy changed.
- No new coordinator, service, global singleton, test-only asset, or runtime
  behavior was introduced.
- No QApplication launch, clean-machine, cross-machine, signing, installer,
  updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Ohm the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Einstein the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — direct tracker reserve
  composition and call sites.
- `docs/adr/0152-operation-reserve-facade-simplification.md`
- `docs/agent-team/reviews/D117-operation-reserve-parent-review.md`
- `docs/agent-team/reviews/D117-operation-reserve-independent-review.md`
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- `OperationTracker` owns only monotonic allocation and lifecycle identity;
  MainWindow remains the owner of busy/status and application policy.
- The existing SessionSaveCoordinator callable contract remains the only
  session-save allocation port; no parallel allocator was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation
  was used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D117-OPERATION-RESERVE-PROBE=PASS` | PASS | Seven direct reserve call sites and direct session-save injection. |
| `D117-OPERATION-BEHAVIOR-BOUNDARY-PROBE=PASS` | PASS | Begin/complete busy/status policy remains present. |
| `D117-CONSTRUCTION-ORDER-PROBE=PASS` | PASS | Tracker initializes before SessionSaveCoordinator composition. |
| `D117-DISPATCH-CALLSITE-PROBE=PASS` | PASS | Existing load/search/settings/recovery coordinator paths remain. |
| `uv run python -m compileall -q src/quillforge/presentation/main_window.py src/quillforge/presentation/session_save_coordinator.py` | PASS | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge/presentation/main_window.py src/quillforge/presentation/session_save_coordinator.py` | PASS | All checks passed. |
| `uv run ruff format --check src scripts` | PASS | Recorded after full repository formatting. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Presentation contract audit passed; no QApplication launch. |
| `D117-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| `D117-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process afterward. |
| `D117-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, register, index, manifest, and dossier bind to D117 identity. |
| `D117-RELEASE-DOSSIER-PROBE=PASS` | PASS | Current dossier is D117-bound and records the expected no-go decision. |
| `D117-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Open runtime/release gates keep the verifier non-zero as required. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Runtime callback interleaving, QApplication startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and
  release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes prove call-site and construction-order shape, not runtime
  callback interleaving or close-time behavior.
- Ohm architecture and Einstein independent review both returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; report
  binding and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S156`, `D117-AC01`.
- Evidence: ADR-0152, D117 source probes, parent/independent review records,
  simplification assessment, static checks, package identity, handoff/index/
  register checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: 3688264AE2422757ED134661FAEAD019C46C6349A7312A96BE7D210130029603
- Size: 38502140 bytes
- Source revision: tree-sha256:abe90cc3fe5812e7ca6825219850c72e6ded64feba5cb3822cc1394d399e65bc
- Manifest: `dist/QuillForge.release.json`

## Disposition

accepted-with-limits: the pure operation-ID forwarding facade is removed while
the canonical tracker and application policy boundaries remain intact.
