# Handoff: 2026-08-10-d15-workspace-search-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d15-workspace-search-surface` |
| Delivery / slice | `D15 / ARCH-06 MainWindow workspace-search surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T10:00:00+08:00` |

## User outcome

The workspace Find in Files dialog now has a focused presentation owner. Its
composition, three semantic routes, activation, locale, busy/cancellation
feedback, and result/error projection are centralized while search execution,
stale-result rejection, root containment, document opening, and notifications
remain under the existing MainWindow/application boundary.

## Scope and boundaries

### In scope

- `WorkspaceSearchSurface` and `WorkspaceSearchSurfaceCallbacks`.
- Existing `WorkspaceSearchDialog` creation, parent lifetime, signal binding,
  non-modal activation, root/locale projection, busy/cancel feedback, and
  result/error/cancelled projection.
- Removal of duplicate workspace panel locale projection and a defensive null
  panel dereference in the directory error path; refresh of the existing
  truncated-directory marker during locale changes.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No `WorkspaceSearchService`, query policy, `TaskRunner`, operation ID,
  generation, cancellation event, stale-result policy, containment check,
  document activation, session barrier, or notification policy moved into the
  surface.
- No regex/indexing/remote search/cross-file replacement feature was added.
- No service locator, dependency-injection framework, singleton, event bus,
  microservice/RPC layer, wholesale MainWindow rewrite, or public plugin
  behavior change.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Find in Files user outcome and result activation acceptance |
| Developer 1 | fixed six-role workflow | Search application ownership and lifecycle boundary |
| Developer 2 | fixed six-role workflow | Qt search surface composition and projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Schrodinger / Luna max | Read-only D15 review; source-level PASS with runtime limits |

## Changed files and modules

- `src/quillforge/presentation/workspace_search_surface.py` — owns search
  dialog composition, semantic routes, activation, and projection.
- `src/quillforge/presentation/main_window.py` — composes the surface and
  retains query, async, cancellation, containment, document, and notification
  behavior; removes duplicate panel locale projection and hardens a null guard.
- `src/quillforge/presentation/workspace_panel.py` — reprojects the existing
  truncated-directory marker when locale changes without clearing the page.
- `docs/adr/0040-main-window-workspace-search-surface.md` — D15 decision and
  invariants.
- `docs/agent-team/reviews/D15-workspace-search-surface-parent-review.md` —
  parent review, independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D15 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `WorkspaceSearchSurfaceCallbacks` carries exactly three semantic intents and
  does not expose services or mutable application state.
- `WorkspaceSearchSurface` owns the dialog object and activation sequence;
  MainWindow remains the only owner of search lifecycle and containment.
- Root changes invalidate the active search before the surface clears stale
  rows, preserving generation/cancellation ordering.
- Locale refresh uses one workspace surface path; the panel keeps its current
  directory page and translates an existing truncation marker.
- Shared checkout writer: Architect. No worktree, unit-test-only asset, or Qt
  launch was used.

## Public-source applicability and embedded gate

This slice is Python/PyQt6, not embedded C/C++ or firmware. The embedded
enterprise workflow and embedded code-review simplifier are **N/A** for
MCU/vendor constraints because no firmware target, SDK, RTOS, ISR/DMA, driver,
protocol, boot, Flash/NVM, power, or hardware was changed. Public architecture
references are engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
No private ByteDance standard, certification, manufacturer requirement, or
release-readiness claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS after final source repair | Compile gate is non-destructive. |
| `uv run ruff check src\quillforge` | PASS after final source repair | Lint gate is non-destructive. |
| `uv run ruff format --check src\quillforge` | PASS after final source repair | Format gate is non-destructive. |
| D15 source boundary/lifecycle probe | PASS by source reasoning; final exact probe recorded below | MainWindow has no dialog construction or external search signal wiring. |
| `scripts/verify_handoff.ps1` | PASS | D15 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | D15 portable candidate rebuilt with root/dist identity `9CA0ACC937DDA93AF5F440A3994C840F226C6A3FDE7A02C2645C84B0797D3977` / `38,378,840` bytes. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Dossier refresh is expected to retain the known report failures and ten open release gates. |

## Independent review

Schrodinger / Luna max was assigned a bounded read-only D15 review with no
write access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** and reported no blocking FAIL.
Qt signal delivery, parent lifetime, focus/activation, double-click, and
compile/runtime limitations remain explicitly unverified.

## Simplification assessment

The extraction removes dialog construction, three external signal-connect
operations, and repeated activation calls from MainWindow. One explicit frozen
callback record replaces those inline details; no second state model or generic
container is introduced. The same review also led to removal of duplicate
workspace locale projection and a defensive null dereference, both preserving
behavior while reducing hidden assumptions.

## Unrun checks and reason

- QApplication/Qt startup, signal delivery, dialog focus/activation,
  destruction, visual locale, search interaction, result double-click,
  screenshots, screen-reader output, and native DPI/font metrics — prohibited
  by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow still owns workspace async/search/session orchestration; future
  slices should extract those only with separate contracts and evidence.
- Runtime search dialog activation, result timing, accessibility, native
  metrics, fonts, and file opening remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D15-AC01`, `S44`.
- Evidence: ADR-0040, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow coordinator slice only after
  D15 package and release-no-go evidence are refreshed; keep runtime and
  external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `9CA0ACC937DDA93AF5F440A3994C840F226C6A3FDE7A02C2645C84B0797D3977` / `38,378,840` bytes; source `tree-sha256:b02a5682e4bc39b31cf831e59442d843286c5a29b5f6910dc6632fa1ebd788b9`.
- Packaging note: portable candidate rebuilt after D15 source edits; it is not
  release approval.

## Disposition

`accepted-with-limits`: D15 workspace-search surface extraction is source-level
behavior-preserving by reviewed ownership and callback/lifecycle invariants;
runtime interaction is unrun, and the remaining architecture/release gates
stay open.
