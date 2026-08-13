# Handoff: 2026-08-10-d16-workspace-projection-closure

| Field | Value |
|---|---|
| ID | `2026-08-10-d16-workspace-projection-closure` |
| Delivery / slice | `D16 / ARCH-07 MainWindow workspace projection-boundary closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T11:00:00+08:00` |

## User outcome

Workspace navigation now has one presentation owner for both user intents and
current application-result projection. MainWindow no longer reaches into the
concrete panel widget, while workspace service calls, cancellation, stale
guards, session restore, containment, and notifications remain unchanged.

## Scope and boundaries

### In scope

- `WorkspaceSurface` semantic `current_path`, loading, directory, and error
  projection methods.
- Removal of MainWindow's `WorkspacePanel` import, `_workspace_panel`
  compatibility property, and direct panel method calls.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No `WorkspaceService`, `TaskRunner`, operation ID, generation, cancellation,
  session barrier, containment rule, document activation, notification, or
  error policy moved into the surface.
- No new workspace behavior, recursive search behavior, plugin behavior,
  service locator, singleton, event bus, dependency-injection framework, or
  wholesale MainWindow rewrite.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Workspace navigation result projection acceptance |
| Developer 1 | fixed six-role workflow | Workspace service and lifecycle boundary |
| Developer 2 | fixed six-role workflow | Workspace surface and widget projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Curie / Luna max | Source-level PASS; runtime limitations recorded below |

## Changed files and modules

- `src/quillforge/presentation/workspace_surface.py` — owns semantic
  current-path/loading/directory/error projection over the panel.
- `src/quillforge/presentation/main_window.py` — uses only WorkspaceSurface
  for workspace projection and retains all workspace/application policy.
- `docs/adr/0041-main-window-workspace-projection-closure.md` — D16 decision.
- `docs/agent-team/reviews/D16-workspace-projection-closure-parent-review.md` —
  parent review, independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D16 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `WorkspaceSurface` is the sole owner of the concrete `WorkspacePanel`
  projection; MainWindow sees only semantic surface methods.
- The four methods are thin delegations and introduce no second directory or
  loading state model.
- Existing operation completion, generation/cancellation, session-restore,
  containment, and notification ordering are retained.
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
| `uv run python -m compileall -q src\quillforge` | PASS before final source/package refresh | Compile gate is non-destructive. |
| `uv run ruff check src\quillforge` | PASS before final source/package refresh | Lint gate is non-destructive. |
| `uv run ruff format --check src\quillforge` | PASS before final source/package refresh | Format gate is non-destructive. |
| D16 dependency/lifecycle probe | PASS by source reasoning; final exact probe recorded below | MainWindow no longer imports or references WorkspacePanel. |
| `scripts/verify_handoff.ps1` | PASS | D16 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D16 portable candidate rebuilt with root/dist identity `22AC2146C2499B0994BF3E93B6705D586C85B1905995214893350DF1F88220E3` / `38,378,407` bytes. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Dossier refresh retains the known report failures and ten open release gates. |

## Independent review

Curie / Luna max was assigned a bounded read-only D16 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. Qt runtime
delivery, widget lifetime, and interactive behavior remain unverified.

## Simplification assessment

The change removes one widget import, one compatibility property, and all
direct panel projection calls from MainWindow. Four thin methods on the
existing WorkspaceSurface make the semantic boundary explicit without adding
state, containers, or policy. No further safe behavior-preserving
simplification is required for this bounded slice.

## Unrun checks and reason

- QApplication/Qt startup, signal delivery, panel lifetime, visual rendering,
  loading/cancel timing, folder/file interaction, screenshots, screen-reader
  output, and native DPI/font metrics — prohibited by the permanent no-launch
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow still owns workspace async/session coordination and remains a
  large coordinator; future slices should keep contracts explicit.
- Runtime workspace projection, native metrics, accessibility, fonts, DPI, and
  file activation remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D16-AC01`, `S45`.
- Evidence: ADR-0041, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D16 package and release-no-go evidence are refreshed; keep runtime
  and external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `22AC2146C2499B0994BF3E93B6705D586C85B1905995214893350DF1F88220E3` / `38,378,407` bytes; source `tree-sha256:3beca260f18203ff06a80de517a26c0fe20e56545d98f6e6aba089afca1423b`.
- Packaging note: portable candidate rebuilt after D16 source edits; it is not
  release approval.

## Disposition

`accepted-with-limits`: D16 closes the workspace widget projection boundary by
reviewed source ownership and lifecycle invariants; runtime interaction is
unrun, and the remaining architecture/release gates stay open.
