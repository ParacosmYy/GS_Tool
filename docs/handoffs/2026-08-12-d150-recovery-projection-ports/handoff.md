# Handoff: 2026-08-12-d150-recovery-projection-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d150-recovery-projection-ports |
| Delivery / slice | D150 / ARCH-136 recovery projection Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T19:00:00+08:00 |

## User outcome

Recovery-writer outcomes now use an explicit immutable callback contract while
preserving deletion, stale-owner handling, dirty/snapshot/content-version
decisions, newer-edit feedback, and failure feedback.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `RecoveryProjectionPorts` contract.
- Saved/failed branch order preservation and MainWindow named wiring.
- Branch, static, compile, package, and traceability evidence.

### Out of scope

- No recovery capture, writer lifecycle, timers, persistence format, delete
  service, close policy, notification text, Qt surface, async worker, or
  runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Anscombe the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Banach the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/recovery_projection_coordinator.py` — named
  Ports contract and preserved branch projection.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D150 scope and status.
- `docs/adr/0198-recovery-projection-ports.md`
- `docs/agent-team/reviews/D150-recovery-projection-ports-parent-review.md`
- `docs/agent-team/reviews/D150-recovery-projection-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only recovery-result branch projection; concrete
  recovery, deletion, tab, notification, writer, and policy ownership remain
  in MainWindow.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D150-RECOVERY-BRANCH-PROBE=PASS`
- `D150-SOURCE-WIRING-PROBE=PASS`
- `D150-PRESENTATION-AUDIT=PASS`
- `D150-COMPILEALL=PASS`
- `D150-RUFF=PASS`
- `D150-FORMAT=PASS`
- `D150-PACKAGE-BUILD=PASS`
- `D150-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native event timing, recovery durability, filesystem behavior,
  font/DPI, accessibility, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove queued callback timing or recovery
  durability under power loss and filesystem pressure.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S202`, `D150-AC01`.
- Evidence: ADR-0198, parent/independent review records, D150 probes, static
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
- SHA-256: `28B29A9906539388762D33749BA18A2FFF3142710AA4C5B20482DEB0D734EAD5`
- Size: `38543581` bytes
- Source revision: `tree-sha256:b76f3bb9fea61c77ba0960bc9faf7bab61123d7fc092faeb78d4f42f76ce1ed8`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: recovery outcome projection now has a named immutable
contract with unchanged safety branches; native durability/timing, runtime,
release, and external evidence gates remain open.
