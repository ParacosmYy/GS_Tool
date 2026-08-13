# Handoff: 2026-08-10-d17-find-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d17-find-surface` |
| Delivery / slice | `D17 / ARCH-08 MainWindow Find surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T12:00:00+08:00` |

## User outcome

Find and Replace now have a focused presentation owner for the bar's layout,
signals, mode, locale, status, and operation feedback. MainWindow continues to
own active-editor behavior, Replace All safety, cancellation/rollback, tab
locking, and error policy.

## Scope and boundaries

### In scope

- `FindSurface` and `FindSurfaceCallbacks`.
- Existing `FindBar` construction, shell placement, six signal routes,
  hide/show-find mode, locale, query/replacement/case, status,
  operation-active, and session-reset projection.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No editor lookup, literal find/replace, `_find_match`, Replace All session,
  content-version guard, cancellation/rollback, tab lock, notification, or
  error policy moved into the surface.
- No new search syntax, multi-file replacement, editor behavior, service
  locator, singleton, event bus, dependency-injection framework, or wholesale
  MainWindow rewrite.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Find/Replace user outcome and safety acceptance |
| Developer 1 | fixed six-role workflow | Editor and Replace All ownership boundary |
| Developer 2 | fixed six-role workflow | FindBar presentation composition |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Gödel / Luna max | Read-only D17 review; bounded result recorded below |

## Changed files and modules

- `src/quillforge/presentation/find_surface.py` — owns FindBar composition,
  semantic routes, shell placement, and projection.
- `src/quillforge/presentation/main_window.py` — composes FindSurface and
  retains all editor/Replace All behavior.
- `docs/adr/0042-main-window-find-surface.md` — D17 decision and invariants.
- `docs/agent-team/reviews/D17-find-surface-parent-review.md` — parent review,
  independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D17 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `FindSurfaceCallbacks` carries six semantic intents and no application
  services or mutable editor state.
- FindSurface owns the concrete bar and shell widget placement; MainWindow
  remains the behavior owner.
- Existing signal payloads, keyboard path, query state, mode, and Replace All
  lifecycle remain source-equivalent.
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
| D17 FindSurface boundary/six-route probe | PASS by source reasoning; final exact probe recorded below | MainWindow has no FindBar construction or external wiring. |
| `scripts/verify_handoff.ps1` | PASS | D17 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D17 portable candidate rebuilt with root/dist identity `CFD8510A81A3E3AC0BB851125EE5DD573344C8F29ECF843984BE654A93C5E4CD` / `38,380,906` bytes. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and external release gates remain open. |

## Independent review

Gödel / Luna max was assigned a bounded read-only D17 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed the six callback routes, shell placement/lifetime, and retained
editor/Replace All policy; runtime interaction remains unverified.

## Simplification assessment

The extraction removes FindBar construction, six external signal-connect
operations, shell placement details, and direct widget projection from
MainWindow. One explicit frozen callback record and thin semantic delegates
replace those details; no editor or Replace All state is duplicated. No
further safe behavior-preserving simplification is required for this slice.

## Unrun checks and reason

- QApplication/Qt startup, signal delivery, focus/keyboard behavior, native
  metrics, visual Find/Replace, Replace All interaction, screenshots,
  screen-reader output, and DPI/font metrics — prohibited by the permanent
  no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices should keep contracts explicit.
- Runtime Find/Replace keyboard/focus behavior, accessibility, native metrics,
  fonts, DPI, and Replace All timing remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D17-AC01`, `S46`.
- Evidence: ADR-0042, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow coordinator slice only after
  D17 package and release-no-go evidence are refreshed; keep runtime and
  external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `CFD8510A81A3E3AC0BB851125EE5DD573344C8F29ECF843984BE654A93C5E4CD` / `38,380,906` bytes; source `tree-sha256:2e765af62414d08b08639d5aba6bcba16bf134fb29c7ced1a90a5f61a89215a7`.
- Packaging note: historical portable candidate was rebuilt after D17 source edits;
  it is not release approval.

## Disposition

`accepted-with-limits`: D17 Find surface extraction is source-level
behavior-preserving by reviewed ownership and callback invariants; runtime
interaction is unrun, and the remaining architecture/release gates stay open.
