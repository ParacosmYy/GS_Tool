# Handoff: 2026-08-10-d14-workspace-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d14-workspace-surface` |
| Delivery / slice | `D14 / ARCH-05 MainWindow workspace surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T09:00:00+08:00` |

## User outcome

QuillForge's workspace navigation dock now has a focused presentation owner.
The dock/panel composition, five semantic navigation routes, and locale
projection are centralized while workspace opening, directory enumeration,
search, session restore, cancellation, and containment policy remain under the
existing MainWindow/application boundary.

## Scope and boundaries

### In scope

- `WorkspaceSurface` and `WorkspaceSurfaceCallbacks` presentation contract.
- `QDockWidget`/`WorkspacePanel` creation, parenting, object name, left-dock
  placement, signal binding, and dock/panel locale synchronization.
- MainWindow delegation with workspace service, TaskRunner, generation,
  cancellation, containment, search/session, notification, and error ownership
  preserved.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No WorkspaceService, workspace search, session barrier, filesystem policy,
  document activation, or recovery behavior moved into the surface.
- No service locator, dependency-injection framework, singleton, event bus,
  microservice/RPC layer, or wholesale MainWindow rewrite.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Workspace navigation/file activation outcome |
| Developer 1 | fixed six-role workflow | Application/workspace ownership boundary |
| Developer 2 | fixed six-role workflow | Qt dock/panel composition and signal projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Kuhn / Luna max | Delayed read-only review returned source-level CONCERNS; D15 records remediation |

## Changed files and modules

- `src/quillforge/presentation/workspace_surface.py` — owns workspace dock/
  panel composition, semantic signal routes, and locale projection.
- `src/quillforge/presentation/main_window.py` — composes the surface and
  retains workspace/search/session async behavior and result projection.
- `docs/adr/0039-main-window-workspace-surface.md` — decision and invariants.
- `docs/specs/enterprise-architecture-migration.md` — Phase 2 workspace
  surface acceptance.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md` — architecture and roadmap projection.
- `tasks/plan.md`, `tasks/todo.md` — D14 tracking.
- `docs/agent-team/reviews/D14-workspace-surface-parent-review.md` — parent
  review, independent-review record, simplification, and validation.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `WorkspaceSurfaceCallbacks` carries five semantic intents and does not expose
  services or mutable application state.
- `WorkspaceSurface` owns the visual dock/panel objects; MainWindow accesses the
  panel only to project current service results.
- Existing signal payload types and callback targets are unchanged.
- Locale changes update dock title and panel labels without replacing the
  visible directory page.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Public-source applicability and embedded gate

This slice is Python/PyQt6, not embedded C/C++ or firmware. The embedded
enterprise workflow and embedded code-review simplifier are N/A for MCU/vendor
constraints because no firmware target, SDK, RTOS, ISR/DMA, driver, protocol,
boot, Flash/NVM, power, or hardware was changed. Public architecture references
are recorded as engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
No private ByteDance standard, certification, manufacturer requirement, or
release-readiness claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Whole-source compile after D14 extraction. |
| `uv run ruff check src\quillforge` | PASS | Whole-source lint after D14 extraction. |
| `uv run ruff format --check src\quillforge` | PASS | Whole-source format check. |
| D14 AST/source boundary and signal-route probe | PASS | Surface owns dock/panel projection; MainWindow retains policy. |
| JSON parse for acceptance/register/index | PASS | D14 IDs/index entry resolve. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index status equality after D14 evidence refresh. |
| `scripts/check.ps1` | PASS | NOTICE, handoff, source, and formatting gates after D14. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Portable candidate rebuilt after D14 source edit. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1 with the known three mechanical report failures and ten open release gates. |

## Independent review

Kuhn / Luna max was assigned a bounded read-only review with no write access,
no Qt launch, and no test creation/run. The initial bounded waits expired
without a conclusion; a delayed completion then returned source-level
**CONCERNS** with no blocking FAIL. It identified duplicate direct panel locale
projection, a stale-language truncated-directory marker, and a defensive null
panel dereference. D15 removes the duplicate call, refreshes the marker, and
hardens the null guard; those repairs are recorded in the D15 review/handoff.

## Simplification assessment

The slice removes dock construction and five signal-connect operations from
MainWindow while introducing one explicit callback record and one surface. It
does not duplicate workspace state, move async policy, or add a container,
singleton, event bus, or speculative abstraction. The panel access property is
a read-only projection over the surface's sole panel owner. No further safe
simplification is required for this bounded change.

## Unrun checks and reason

- Qt startup, signal delivery, QWidget destruction/reparenting, dock placement,
  screenshots, keyboard/file interaction, screen-reader output, and runtime
  visual acceptance — prohibited by the active no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow still owns workspace async/search/session orchestration; future
  slices should extract those only with separate contracts and evidence. The
  delayed source concerns were remediated in D15; the D14 package remains a
  historical artifact.
- Runtime-native dock metrics, callback timing, accessibility rendering,
  installed fonts, DPI, and file activation remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D14-AC01`, `S43`.
- Evidence: ADR-0039, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded workspace/search/session coordinator slice
  only after D14 package and release-no-go evidence are refreshed; keep runtime
  and external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `7695A65EECCFA2B40D1FA4485EED11CFEEB326C408DF786AB32291DB2E099E27` / `38,375,608` bytes; source `tree-sha256:cfc2417e01447cc9f62a652578617b16a0b8bbd9eb3a84b4db0e2e2593a1747c`.
- Packaging note: portable package evidence is not release approval.

## Disposition

`accepted-with-limits`: D14 workspace surface extraction is source-level
behavior-preserving by reviewed ownership and signal invariants; the
independent review is explicitly recorded as no-conclusion, runtime interaction
is unrun, and the remaining architecture/release gates stay open.
