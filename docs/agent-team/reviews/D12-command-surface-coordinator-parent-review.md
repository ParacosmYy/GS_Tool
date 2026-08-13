# D12 / ARCH-03 parent review: MainWindow command-surface coordinator

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | Presentation-only extraction of menu and command-rail projection |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

QuillForge continues the enterprise migration with a focused presentation
coordinator. The menu and command rail can now evolve in one module while the
MainWindow retains document/application callbacks and plugin lifecycle seams.
This advances the modern UI without moving business policy into Qt widgets.

## Public-source applicability

D12 inherits the public-source applicability record from D11:

- [CloudWeGo About](https://www.cloudwego.io/about/) — independently usable,
  extensible components and reliability concerns are transferable engineering
  references.
- [CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/)
  — incremental, externally verifiable evolution is used as a process
  reference.
- [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/)
  — interface-oriented extension and composable options inform the explicit
  `ToolbarActionSpec`/callback boundary.

These are public engineering references, not ByteDance private standards,
manufacturer requirements, certification evidence, or a claim of internal
implementation equivalence.

## Architecture decision applied

- `CommandSurface` owns menu/toolbar Qt widgets, action rebuilding, and locale
  retranslation.
- `ToolbarActionSpec` carries only callback and visual metadata needed to build
  the command rail.
- `MainWindow` owns core command registration, document/workspace/plugin
  behavior, and the compatibility method `refresh_command_menus()`.
- The coordinator reads `CommandRegistry` and `Locale` through explicit
  presentation/application boundaries and imports no infrastructure.
- Existing ordering is preserved: register commands → create menus/refresh →
  create command rail → create status rail → retranslate shell.

## Changed files and modules

- `src/quillforge/presentation/command_surface.py` — new focused projection
  coordinator and immutable toolbar spec.
- `src/quillforge/presentation/main_window.py` — delegates menu/toolbar
  projection and locale refresh while retaining behavior callbacks.
- `docs/adr/0037-main-window-command-surface.md` — decision and invariants.
- `docs/specs/enterprise-architecture-migration.md` — Phase 2 acceptance.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md` — architecture and roadmap
  projections.
- `tasks/plan.md`, `tasks/todo.md` — ARCH-03 progress.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — D12 evidence projections.

## Fixed-role input and ownership

| Role | Owner / agent | Contribution | Disposition |
|---|---|---|---|
| Architect | parent | Defined contract, integrated source, and owns final evidence | accepted |
| Project Manager | fixed six-role workflow | D12 dependencies and open release gates | incorporated |
| Product | fixed six-role workflow | Preserve user-visible command behavior while reducing coordinator coupling | incorporated |
| Developer 1 | fixed six-role workflow | Checked application/presentation ownership and callback boundary | incorporated |
| Developer 2 | fixed six-role workflow | Checked Qt object ownership, action order, and locale projection | incorporated |
| QA | fixed six-role workflow | Static, package, no-launch, and provenance verification matrix | incorporated |
| Independent reviewer | Poincare / Luna max | Read-only review of command surface and MainWindow integration | no conclusion; bounded wait expired and reviewer was closed |

## Simplification assessment

The extraction removes approximately 100 lines of menu/toolbar widget assembly
from MainWindow without introducing a service locator, dependency-injection
container, or duplicate command registry. The stable MainWindow delegation
methods remain intentionally small because they preserve the existing lifecycle
surface. `CommandSurface` keeps Qt ownership local and makes the old ordering
explicit. No additional behavior-preserving simplification was identified
before the independent review.

## Independent review result

Poincare / Luna max was assigned a bounded, read-only review of
`command_surface.py` and its MainWindow integration. No conclusion was returned
within the bounded wait window; the reviewer was closed and no PASS is claimed.
The parent review is therefore the controlling record with this explicit
no-conclusion disposition.

## Authorized non-destructive verification

- `uv run python -m compileall -q src\quillforge` — PASS after source change.
- `uv run ruff check src\quillforge` — PASS after source change.
- `uv run ruff format --check src\quillforge` — PASS after source change.
- D12 source boundary probe — PASS: MainWindow delegates projection, the new
  module owns projection state, and no infrastructure import is introduced.
- `scripts/verify_handoff.ps1` — PASS after final D12 handoff/index synchronization.
- `scripts/check.ps1` — PASS after final D12 evidence synchronization.
- `scripts/package.ps1` — PASS after D12 source package refresh.
- `scripts/verify_release_handoff.ps1` — expected NO-GO; it must retain the
  known three mechanical failures and ten open external gates.

## Unrun checks and reason

- QApplication/Qt startup, screenshots, interactive menu/toolbar behavior,
  native accessibility output, and runtime visual acceptance — prohibited by
  the project no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, signing, installer/update, deployment, and hardware checks —
  not authorized and outside this local presentation slice.

## Known risks and limits

- MainWindow remains a large coordinator; document, workspace, recovery,
  session, and plugin extraction are future bounded slices.
- Source probes cannot prove native Qt metrics or runtime keyboard behavior
  without authorized execution.
- Package provenance must be refreshed after the source change; prior D11/UI-14
  artifact identities are historical.
- D7/D8 and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D12-AC01`, `S41`.
- Evidence: ADR-0037, command-surface source, MainWindow source, architecture
  docs, this review, D12 handoff, task plan, static probes, package manifest,
  and release dossier.

## Next owner and next action

- Owner: Architect.
- Action: integrate Poincare's bounded conclusion, package the source slice,
  then plan the next MainWindow coordinator boundary.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `0339FF021E0F8184D2274774C52614DD1D45D944B47A1494EAFD3020F3DC3164` / `38,369,154` bytes; source `tree-sha256:79807e72fcff9edd71008f79e86909a642ef329bdd46ef91e4ed759fd07d484d`.
- Packaging note: package evidence is not release approval.

## Disposition

`accepted-with-limits`: ARCH-03 source extraction is implemented and awaits
independent review/package closure. Full architecture migration, runtime proof,
and external release gates remain open.
