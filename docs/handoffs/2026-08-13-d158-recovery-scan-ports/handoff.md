# Handoff: 2026-08-13-d158-recovery-scan-ports

| Field | Value |
|---|---|
| ID | 2026-08-13-d158-recovery-scan-ports |
| Delivery / slice | D158 / ARCH-145 recovery-scan Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-13T00:25:00+08:00 |

## User outcome

Recovery inventory result projection now exposes its existing stale,
invalid, empty, candidate, startup, and failure boundary through named
immutable Ports. Startup continuation and candidate order remain explicit.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `RecoveryScanPorts` contract.
- Tracker stale guard, inventory validation, candidate/empty projection, and
  startup continuation preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No RecoveryService, scan worker, candidate model, startup restore policy,
  filesystem behavior, notification wording, Qt surface, locale/theme/motion
  projection, close policy, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Kant the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Lorentz the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/recovery_scan_coordinator.py` — frozen/slotted
  named Ports and preserved scan result projection.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D158 scope and status.
- `docs/adr/0207-recovery-scan-ports.md`
- `docs/agent-team/reviews/D158-recovery-scan-ports-parent-review.md`
- `docs/agent-team/reviews/D158-recovery-scan-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only scan result classification/projection; Recovery-
  Service, scan worker, restore state, filesystem, notification, startup,
  close, and policy ownership remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D158-RECOVERY-SCAN-BRANCH-PROBE=PASS`
- `D158-STALE-INVALID-EMPTY-CANDIDATE-STARTUP-FAILURE-PROBE=PASS`
- `D158-PORTS-IMMUTABILITY-PROBE=PASS`
- `D158-SOURCE-WIRING-PROBE=PASS`
- `D158-QT-FREE-CONTRACT-PROBE=PASS`
- `D158-PRESENTATION-AUDIT=PASS`
- `D158-COMPILEALL=PASS`
- `D158-RUFF=PASS`
- `D158-FORMAT=PASS`
- `D158-PACKAGE-BUILD=PASS`
- `D158-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native scan timing, startup scheduling, filesystem behavior,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback timing relative to native scan
  delivery or startup scheduling.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S211`, `D158-AC01`.
- Evidence: ADR-0207, parent/independent review records, D158 probes, static
  checks, package manifest, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the Ports-contract change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `1C01FB2E5AE1CBE295D34E37CB3C8357C10D7839B1B1AF0CFDD762BD8E4DE56B`
- Size: `38546847` bytes
- Source revision: `tree-sha256:72a3c626065ca00ab6aa40bb173092561e5347e45da70a327585f73c191230b5`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: recovery-scan projection now has a named immutable
contract with unchanged stale/invalid/empty/candidate/startup/failure
behavior; native timing, scheduling, runtime, release, and external evidence
gates remain open.
