# Handoff: 2026-08-12-d154-settings-save-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d154-settings-save-ports |
| Delivery / slice | D154 / ARCH-141 settings-save Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T23:30:00+08:00 |

## User outcome

Settings-save result projection now exposes its existing stale/invalid/valid/
failure boundary through named immutable Ports. Stale callbacks remain silent;
invalid, valid, and matching failure paths preserve their existing projection.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `SettingsSavePorts` contract.
- Tracker classification and result projection preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No settings validation/persistence, dialog controls, theme/locale/font/motion
  projection, settings tracker state, worker dispatch, notification wording,
  Qt surface, or runtime-startup change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Bernoulli the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Meitner the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/settings_save_coordinator.py` — frozen/slotted
  named Ports and preserved result classification.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D154 scope and status.
- `docs/adr/0203-settings-save-ports.md`
- `docs/agent-team/reviews/D154-settings-save-ports-parent-review.md`
- `docs/agent-team/reviews/D154-settings-save-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only result classification and projection callbacks;
  settings service, validation, persistence, visual refresh, transition,
  notification, worker, and close policy remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D154-SETTINGS-BRANCH-PROBE=PASS`
- `D154-STALE-INVALID-VALID-FAILURE-PROBE=PASS`
- `D154-PORTS-IMMUTABILITY-PROBE=PASS`
- `D154-SOURCE-WIRING-PROBE=PASS`
- `D154-QT-FREE-CONTRACT-PROBE=PASS`
- `D154-PRESENTATION-AUDIT=PASS`
- `D154-COMPILEALL=PASS`
- `D154-RUFF=PASS`
- `D154-FORMAT=PASS`
- `D154-PACKAGE-BUILD=PASS`
- `D154-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native event timing, settings filesystem durability, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release owner checks — prohibited or outside current
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove callback timing relative to queued Qt
  delivery or settings filesystem durability.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S207`, `D154-AC01`.
- Evidence: ADR-0203, parent/independent review records, D154 probes, static
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
- SHA-256: `C0AD8F26EBE75BAAC577322F65C09EA69F342D6B2D171ED6C279ED984CE90797`
- Size: `38542726` bytes
- Source revision: `tree-sha256:42adbabf07c46b276fd9aaf804f7c86f7222610bfdc60e48edb36c8e0b886354`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: settings-save result projection now has a named
immutable contract with unchanged stale/invalid/valid/failure behavior; native
timing, durability, runtime, release, and external evidence gates remain open.
