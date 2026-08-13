# Handoff: 2026-08-12-d156-plugin-runtime-ports

| Field | Value |
|---|---|
| ID | 2026-08-12-d156-plugin-runtime-ports |
| Delivery / slice | D156 / ARCH-143 plugin-runtime Ports contract |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T23:55:00+08:00 |

## User outcome

Plugin runtime status/control projection now exposes its existing presentation
callbacks through named immutable Ports. Runtime, trust, permission, and
enablement policy remain in their existing application boundaries.

## Scope and boundaries

### In scope

- Frozen/slotted Qt-free `PluginRuntimePorts` contract.
- Failure, unavailable, busy, exception, success, and status order
  preservation.
- MainWindow named wiring, source, inline, static, compile, package, and
  traceability evidence.

### Out of scope

- No plugin runtime protocol, trust/permission policy, enablement persistence,
  external plugin host, catalog governance, command registry, notification
  wording, Qt surface, locale/theme/motion projection, or runtime-startup
  change.
- No QApplication/EXE launch, native rendering, clean-machine, cross-machine,
  signing, installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Maxwell the 5th / Luna max | `NO_CONCLUSION` after bounded window; no architecture PASS |
| Independent review | Erdos the 5th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/plugin_runtime_coordinator.py` — frozen/slotted
  named Ports and preserved runtime control projection.
- `src/quillforge/presentation/main_window.py` — named Ports construction only.
- `tasks/plan.md` and `tasks/todo.md` — bounded D156 scope and status.
- `docs/adr/0205-plugin-runtime-ports.md`
- `docs/agent-team/reviews/D156-plugin-runtime-ports-parent-review.md`
- `docs/agent-team/reviews/D156-plugin-runtime-ports-independent-review.md`

## Decisions and constraints

- The coordinator owns only presentation sequencing; runtime, trust,
  permission, enablement, catalog/host, command, notification, and close
  policy remain outside it.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D156-PLUGIN-RUNTIME-BRANCH-PROBE=PASS`
- `D156-FAILURE-UNAVAILABLE-BUSY-EXCEPTION-SUCCESS-PROBE=PASS`
- `D156-PORTS-IMMUTABILITY-PROBE=PASS`
- `D156-SOURCE-WIRING-PROBE=PASS`
- `D156-QT-FREE-CONTRACT-PROBE=PASS`
- `D156-PRESENTATION-AUDIT=PASS`
- `D156-COMPILEALL=PASS`
- `D156-RUFF=PASS`
- `D156-FORMAT=PASS`
- `D156-PACKAGE-BUILD=PASS`
- `D156-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` and no-launch/traceability checks.

## Unrun checks and reason

- Architect and independent conclusions — bounded child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication/native plugin lifecycle timing, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static/inline evidence cannot prove native plugin lifecycle timing or
  runtime-side permission/enablement interleavings.
- The candidate remains unsigned and release remains NO-GO while report,
  clean-machine, legal, installer/update, and release-owner gates are open.

## Acceptance and evidence IDs

- Acceptance: `S209`, `D156-AC01`.
- Evidence: ADR-0205, parent/independent review records, D156 probes, static
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
- SHA-256: `8393D4426D77E20AB5BD136B8E00D7E4ABAFD42248FD890F5159B36EE180DFB8`
- Size: `38545918` bytes
- Source revision: `tree-sha256:3d2b2c24fe48c694c271e33e82514aad827fd7d2bfc39b6f11938720f60f5a32`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: plugin runtime presentation now has a named immutable
contract with unchanged failure/guard/exception/success/status ordering;
native lifecycle, runtime, release, and external evidence gates remain open.
