# Handoff: 2026-08-12-d149-settings-save-projection-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d149-settings-save-projection-ports |
| Delivery / slice | D149 / ARCH-135 settings-save projection Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T18:00:00+08:00 |

## User outcome

The settings-save projection path now exposes its existing five-step order
through a typed, named contract. Settings application, locale refresh, editor
refresh, motion transition, and success feedback remain behaviorally ordered
as before.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `SettingsSaveProjectionPorts` contract.
- Coordinator order preservation and MainWindow named wiring.
- Contract, static, compile, package, and traceability evidence.

### Out of scope

- No settings validation/persistence, dialog controls, locale catalog, theme
  tokens, editor behavior, animation implementation, async worker, close
  policy, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine,
  cross-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Avicenna the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Nietzsche the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/settings_save_projection_coordinator.py` —
  frozen/slotted named Ports and preserved projection order.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D149 scope and status.
- `docs/adr/0197-settings-save-projection-ports.md`
- `docs/agent-team/reviews/D149-settings-save-projection-ports-parent-review.md`
- `docs/agent-team/reviews/D149-settings-save-projection-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only the order contract; concrete settings, editor,
  Qt, animation, notification, and service ownership remain in MainWindow.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D149-PROJECTION-ORDER-PROBE=PASS`
- `D149-SOURCE-WIRING-PROBE=PASS`
- `D149-PRESENTATION-AUDIT=PASS`
- `D149-COMPILEALL=PASS`
- `D149-RUFF=PASS`
- `D149-FORMAT=PASS`
- `D149-PACKAGE-BUILD=PASS`
- `D149-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native event timing, theme/editor rendering, font/DPI,
  accessibility, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback timing relative to queued Qt
  delivery or native theme/font rendering.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S201`, `D149-AC01`.
- Evidence: ADR-0197, parent/independent review records, D149 probes, static
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
- SHA-256: `4B4591A074B985F36ABEFFBF1B1B4948FC030A028131D233F97264A5EB2BD328`
- Size: `38543804` bytes
- Source revision: `tree-sha256:cc9dd570ec8b4877f365c4d7c5e08136b86a4520714e186f342fd7ed628aeb75`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: settings-save projection now has a named immutable
contract with unchanged ordering; native event/rendering, runtime, release,
and external evidence gates remain open.
