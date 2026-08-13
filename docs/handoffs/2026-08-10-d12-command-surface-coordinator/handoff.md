# Handoff: 2026-08-10-d12-command-surface-coordinator

| Field | Value |
|---|---|
| ID | `2026-08-10-d12-command-surface-coordinator` |
| Delivery / slice | `D12 / ARCH-03 MainWindow command-surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T07:00:00+08:00` |

## User outcome

QuillForge's menu and command rail now have a focused presentation owner. The
shell can continue becoming modern and extensible without making MainWindow
own every Qt projection detail or moving application behavior into the visual
layer.

## Scope and boundaries

### In scope

- `CommandSurface` and `ToolbarActionSpec` presentation boundary.
- MainWindow delegation while preserving command registration and callbacks.
- Menu/toolbar ordering, shortcuts, icons, separator, plugin refresh, and locale
  retranslation source contracts.
- ADR, architecture/spec/roadmap/task updates, fixed-role review, handoff,
  package provenance, and release no-go synchronization.

### Out of scope

- No document, workspace, recovery, session, or plugin coordinator extraction.
- No service locator, dependency-injection framework, microservice/RPC layer,
  or public command behavior change.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | User-visible command behavior and extensibility outcome |
| Developer 1 | fixed six-role workflow | Application/presentation ownership boundary |
| Developer 2 | fixed six-role workflow | Qt ownership, action ordering, and locale projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Poincare / Luna max | Read-only command-surface integration review; no conclusion in bounded wait, reviewer closed |

## Changed files and modules

- `src/quillforge/presentation/command_surface.py` — owns menu/toolbar Qt
  projection, action refresh, and locale retranslation.
- `src/quillforge/presentation/main_window.py` — retains core command
  registration and delegates the existing refresh/retranslate surface.
- `docs/adr/0037-main-window-command-surface.md` — decision and invariants.
- `docs/specs/enterprise-architecture-migration.md` — Phase 2 acceptance.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md` — architecture projections.
- `tasks/plan.md`, `tasks/todo.md` — D12 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `CommandSurface` consumes the application-owned `CommandRegistry` but does
  not register commands or own application policy.
- `ToolbarActionSpec` is presentation metadata plus an explicit callback; it
  does not expose widgets or infrastructure to application services.
- Qt actions remain parented to MainWindow, matching prior lifetime ownership.
- The old `MainWindow.refresh_command_menus()` seam remains available to the
  composition/plugin lifecycle.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Whole-source compile after extraction. |
| `uv run ruff check src\quillforge` | PASS | Whole-source lint after extraction. |
| `uv run ruff format --check src\quillforge` | PASS | Whole-source format check. |
| D12 source boundary probe | PASS | Projection state moved; MainWindow callbacks and seam retained. |
| `scripts/verify_handoff.ps1` | PASS | Final D12 handoff/index status equality. |
| `scripts/check.ps1` | PASS | Final D12 evidence and formatting gate. |
| `scripts/package.ps1` | PASS | D12 source package rebuilt through PowerShell Core. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Preserve three known failures and ten open gates. |

## Independent review

Poincare / Luna max was assigned a bounded read-only review. No conclusion was
returned within the bounded wait window; the reviewer was closed and no child
result is assumed to be PASS. The parent review records this explicitly.

## Simplification assessment

The slice removes command-surface widget assembly from MainWindow while keeping
one compatibility seam and one source of command truth. It adds no container,
duplicate registry, or speculative abstraction. The result is a smaller,
reviewable presentation boundary with the same callback/order contract.

## Unrun checks and reason

- Qt startup, screenshots, runtime keyboard/menu behavior, and screen-reader
  output — prohibited by the active no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains large; only ARCH-03 command projection is extracted.
- Runtime-native style metrics and accessibility rendering remain unproven.
- D7/D8 and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D12-AC01`, `S41`.
- Evidence: specification, ADR-0037, source modules, architecture docs, parent
  review, this handoff, task records, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: integrate the independent conclusion and package identity, then plan
  the next bounded MainWindow/application coordinator slice.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `0339FF021E0F8184D2274774C52614DD1D45D944B47A1494EAFD3020F3DC3164` / `38,369,154` bytes; source `tree-sha256:79807e72fcff9edd71008f79e86909a642ef329bdd46ef91e4ed759fd07d484d`.
- Packaging note: portable package evidence is not release approval.

## Disposition

`accepted-with-limits`: D12 command-surface extraction is implemented with
source-level behavior preservation; independent review/package closure and the
remaining enterprise/runtime work are still open.
