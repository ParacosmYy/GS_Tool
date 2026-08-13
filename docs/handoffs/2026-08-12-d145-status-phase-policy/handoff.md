# Handoff: 2026-08-12-d145-status-phase-policy

| Field | Value |
|---|---|
| ID | 2026-08-12-d145-status-phase-policy |
| Delivery / slice | D145 / ARCH-129 status-phase policy |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T12:00:00+08:00 |

## User outcome

The shell status phase now has an explicit deterministic policy boundary. Busy
or retained background work remains visually `WORKING`; a clean background
with a dirty active document remains `ATTENTION`; otherwise the shell is
`READY`. The Qt status surface remains the rendering owner.

## Scope and boundaries

### In scope

- Pure `StatusPhase` contract and `StatusPhaseInput`.
- Qt-free `StatusPhaseCoordinator` and MainWindow `_sync_status_surface`
  projection.
- Reusing the pure phase contract in the status rail/surface.
- Static precedence, dependency, compile, package, and traceability evidence.

### Out of scope

- No change to TaskRunner, trackers, document state, notification messages,
  error projection, close policy, recovery, locale copy, QSS, or status widgets.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Archimedes the 5th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Harvey the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/status_phase_contract.py` — pure phase type.
- `src/quillforge/presentation/status_phase_coordinator.py` — pure priority policy.
- `src/quillforge/presentation/status_bar.py` — consumes the canonical phase type.
- `src/quillforge/presentation/status_surface.py` — consumes the canonical phase type.
- `src/quillforge/presentation/main_window.py` — passes concrete facts to the coordinator.
- `tasks/plan.md` and `tasks/todo.md` — bounded D145 scope and status.
- `docs/adr/0191-status-phase-policy.md`
- `docs/agent-team/reviews/D145-status-phase-policy-parent-review.md`
- `docs/agent-team/reviews/D145-status-phase-policy-independent-review.md`

## Decisions and constraints

- The new modules are framework-neutral; Qt remains in `status_bar.py`,
  `status_surface.py`, and MainWindow only.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D145-STATUS-PHASE-PROBE=PASS`
- `D145-QT-FREE-PROBE=PASS`
- `D145-COMPILEALL=PASS`
- `D145-RUFF=PASS`
- `D145-FORMAT=PASS`
- `D145-CHECK=PASS`
- `D145-VERIFY-HANDOFF=PASS`
- `D145-PACKAGE-BUILD=PASS`
- `D145-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native status-bar rendering, queued event timing, font/DPI,
  accessibility, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static Qt-free precedence evidence cannot prove native queued ordering or
  final status-bar rendering on every Windows font/DPI combination.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S195`, `D145-AC01`.
- Evidence: ADR-0191, parent/independent review records, D145 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `1ED2AEE6E31F46C15E4793AECC275EB9F1044FC479945A5543FCAF3BDCC085E3`
- Size: `38538238` bytes
- Source revision: `tree-sha256:0daa277e72dbce922b4017a603fe3718aed5dd2d96ae777a1749232c93ed43a6`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: the status phase policy is explicit and Qt-free;
native rendering, runtime, release, and external evidence gates remain open.

