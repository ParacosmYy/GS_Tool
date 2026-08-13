# Handoff: 2026-08-10-d79-plugin-runtime-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d79-plugin-runtime-coordinator` |
| Delivery / slice | `D79 / ARCH-54 plugin runtime coordinator boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Registered-plugin failure, status, and enable/disable orchestration now has a
focused Qt-free coordinator. The shell still owns busy/close/composition
policy, and the application `PluginRuntime` remains the only trust,
enablement, activation, and runtime-security authority.

## Scope and boundaries

### In scope

- Qt-free `PluginRuntimeCoordinator` for failure, status, and lifecycle
  projection.
- Typed runtime view, busy, command-refresh, and notification seams.
- MainWindow composition wiring, lazy PluginSurface callbacks, and event
  subscription migration.
- Preservation of existing runtime behavior and shared plugin close gates.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No PluginRuntime implementation, trust policy, enablement persistence,
  activation, external execution, host, catalog, or application contract
  change.
- No MainWindow wholesale rewrite or new asynchronous framework.
- No native runtime, screenshot, accessibility, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Epicurus the 3rd / Luna max | Read-only boundary consultation; `NO_CONCLUSION` after bounded window |
| Independent review | Hegel the 3rd / Luna max | Read-only source review; `NO_CONCLUSION` after bounded window |
| Parent | Architect | Sole writer, integration, source review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/plugin_runtime_coordinator.py` — Qt-free
  runtime failure/status/enablement orchestration.
- `src/quillforge/presentation/main_window.py` — composition wiring, lazy
  runtime callbacks, event subscription, and removal of five callbacks.
- D79 ADR/reviews/handoff and synchronized delivery records.

## Decisions and constraints

- MainWindow retains the document-operation busy predicate and shared plugin
  operation close gates; the coordinator receives only a predicate.
- `PluginRuntime` remains the application-owned runtime policy and security
  boundary. The presentation coordinator cannot grant execution authority.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D79 runtime-boundary probe | `PASS` | Wiring, removed methods, callback order, runtime policy, and close gates. |
| D79 Qt-free coordinator probe | `PASS` | Bare import does not load PyQt6. |
| D79 RED precondition probe | `PASS` | Confirmed the five callbacks/subscription before extraction. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation source. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_handoff.ps1` | `PASS` | Final D79 records and indexed handoff are synchronized. |
| `scripts\check.ps1` | `PASS` | Final repository checks passed after synchronization. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native Qt rendering, event interleaving, dialog interaction, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes do not prove native Qt callback timing or runtime
  manager behavior on this machine.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D79-AC01`, `S108`.
- Evidence: ADR-0104, D79 boundary/Qt-free probes, parent/independent review
  records, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: synchronize D79 delivery records, run handoff/repository checks,
  then continue the next bounded MainWindow/application coordinator slice or
  obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `DDE74DF5C8604B243AD2C3BEC2C00CDD436C6B6F6CA5B237CA28B57CE3ADC154` / `38,455,612` bytes.
- Source revision: `tree-sha256:ff7a8fa8f45392c8166135931ebad8d43fb53d785c91e75ab091932e8b09da19`.

## Disposition

`accepted-with-limits`: runtime presentation orchestration is isolated behind a
Qt-free typed boundary and the package identity is recorded, while native
runtime and release gates remain open.
