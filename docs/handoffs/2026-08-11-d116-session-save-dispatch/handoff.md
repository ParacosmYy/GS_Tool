# Handoff: 2026-08-11-d116-session-save-dispatch

| Field | Value |
|---|---|
| ID | 2026-08-11-d116-session-save-dispatch |
| Delivery / slice | D116 / ARCH-90 session-save dispatch callback boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T03:55:00+08:00 |

## User outcome

Latest-wins session persistence now binds the existing TaskRunner operation
callbacks through the Qt-free SessionSaveCoordinator. MainWindow retains the
SessionService operation factory and all persistence/session policy.

## Scope and boundaries

### In scope

- Add typed SessionSaveOperation/Success/Failure/Dispatcher contracts.
- Add coordinator `submit(...)` and bind it from `drain()` after tracker
  admission.
- Inject the existing TaskRunner callable and guarded SessionService operation
  factory from MainWindow.
- Preserve valid, invalid, failure, stale, queued latest-wins, notification,
  and synchronous dispatcher-exception behavior.

### Out of scope

- No SessionSaveTracker, SessionService, session schema/store, debounce timer,
  snapshot capture, startup restore, notification, or close-policy behavior
  changed.
- No new async framework, runner adapter, retry, event bus, or test-only asset
  was introduced.
- No QApplication launch, filesystem durability workload, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Mill the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Curie the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/session_save_coordinator.py` — typed dispatch
  contract and coordinator submit seam.
- `src/quillforge/presentation/main_window.py` — inject dispatcher and save
  operation factory; remove direct callback-binding helper.
- `docs/adr/0147-session-save-dispatch-callback-boundary.md`
- `docs/agent-team/reviews/D116-arch-90-session-save-parent-review.md`
- `docs/agent-team/reviews/D116-arch-90-session-save-independent-review.md`
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- SessionSaveCoordinator remains the single latest-wins callback lifecycle
  owner; it receives only callable ports and domain snapshots.
- MainWindow retains SessionService, snapshot capture, debounce, startup
  restore, notification, persistence, and close-policy ownership.
- Tracker admission occurs before dispatch and dispatcher exceptions propagate;
  no retry or hidden recovery policy was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D116-SESSION-SAVE-DISPATCH-PROBE=PASS | PASS | Valid, queued latest-wins, invalid, failure, stale callback, notification, and dispatcher-exception paths covered. |
| D116-SOURCE-DEPENDENCY-PROBE=PASS | PASS | Typed dispatcher/submit present; coordinator remains Qt-free/service-free; MainWindow no longer has `_submit_session_save`. |
| `uv run python -m compileall -q src scripts` | PASS | Static compilation only; no QApplication launch. |
| `uv run ruff check src scripts` | PASS | All checks passed. |
| `uv run ruff format --check src scripts` | PASS | All files are formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Presentation contracts passed after D116 source and traceability synchronization; no QApplication launch. |
| D116-PACKAGE-IDENTITY-PROBE=PASS | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| D116-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no QuillForge process was present afterward. |
| D116-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, release manifest, and current dossier bind to D116 identity. |
| D116-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is D116-bound and records the expected no-go decision. |
| D116-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the verifier non-zero as required by the evidence boundary. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Native TaskRunner timing, actual session-store durability, QApplication
  startup, accessibility, DPI, fonts, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline probes prove callback shape and lifecycle ordering, not native
  scheduling, filesystem durability, or power-loss behavior.
- Mill architecture and Curie independent review both returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; report
  binding and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S151`, `D116-AC01`.
- Evidence: ADR-0147, D116 source/lifecycle probes, parent/independent review
  records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and root `QuillForge.exe`
- SHA-256: 69A98FDF5225D0BE9A61E44AA02B06926930D527D2653DFF82B03F722718FFF2
- Size: 38497165 bytes
- Source revision: tree-sha256:9b1f57bb546145c7a3476c270bb6fa6e289722252c0fafafac24bc3c0772c9ac
- Manifest: `dist/QuillForge.release.json`

## Disposition

accepted-with-limits: latest-wins session-save callback binding is consolidated
behind the existing Qt-free coordinator while native timing, durability,
runtime, and enterprise release gates remain open.
