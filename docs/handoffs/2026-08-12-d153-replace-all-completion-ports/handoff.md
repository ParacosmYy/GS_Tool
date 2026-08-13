# Handoff: 2026-08-12-d153-replace-all-completion-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d153-replace-all-completion-ports |
| Delivery / slice | D153 / ARCH-140 Replace All completion Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T23:00:00+08:00 |

## User outcome

Replace All completion cleanup now exposes its existing lifecycle-release
boundary through named immutable Ports. Current jobs release editor/tab and
operation state in the established order; stale jobs remain silent.

## Scope and boundaries

### In scope

- Frozen/slotted generic Qt-free `ReplaceAllCompletionPorts` contract.
- Tracker stale guard and completion callback-order preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No ReplaceAllSession algorithm, slice timer, editor mutation, rollback,
  cancellation policy, status wording, locale, theme, motion, tab model, or
  runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Carver the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Zeno the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/replace_all_completion_coordinator.py` —
  frozen/slotted generic Ports and preserved finish order.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D153 scope and status.
- `docs/adr/0202-replace-all-completion-ports.md`
- `docs/agent-team/reviews/D153-replace-all-completion-ports-parent-review.md`
- `docs/agent-team/reviews/D153-replace-all-completion-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only stale-guarded lifecycle release and callback
  order; editor, FindSurface, tab, operation, session, rollback, timer,
  notification, and policy ownership remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D153-COMPLETION-ORDER-PROBE=PASS`
- `D153-STALE-JOB-PROBE=PASS`
- `D153-PORTS-IMMUTABILITY-PROBE=PASS`
- `D153-SOURCE-WIRING-PROBE=PASS`
- `D153-QT-FREE-CONTRACT-PROBE=PASS`
- `D153-PRESENTATION-AUDIT=PASS`
- `D153-COMPILEALL=PASS`
- `D153-RUFF=PASS`
- `D153-FORMAT=PASS`
- `D153-PACKAGE-BUILD=PASS`
- `D153-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native event timing, editor rollback, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback timing relative to queued Qt
  delivery or editor rollback behavior.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S206`, `D153-AC01`.
- Evidence: ADR-0202, parent/independent review records, D153 probes, static
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
- SHA-256: `9E85DA8F96FE9CABAE5D37A0FB623697A398315135D3676776C90D932BF5BA5E`
- Size: `38544791` bytes
- Source revision: `tree-sha256:d4532e3adc81ad32372765da2fedf9996d74eee368f98e207f821d087755f8fb`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: Replace All completion now has a named immutable
contract with unchanged stale and release ordering; native event/rollback,
runtime, release, and external evidence gates remain open.
