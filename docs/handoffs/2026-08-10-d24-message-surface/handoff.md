# Handoff: 2026-08-10-d24-message-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d24-message-surface` |
| Delivery / slice | `D24 / ARCH-15 MainWindow message surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T17:00:00+08:00` |

## User outcome

Common save-before-close, About, and recoverable error dialogs now use one
localized presentation seam. The editor still owns the close/save consequence,
asynchronous save, error lifecycle phase, and status projection, while the
surface is ready for consistent future visual refinement.

## Scope and boundaries

### In scope

- `MessageSurface` parentage, locale, common QMessageBox composition, and
  typed Save/Discard/Cancel projection.
- MainWindow delegation while retaining dirty checks, Save As selection,
  asynchronous save, tab removal, error phase, and status synchronization.
- D24 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No save, close, document, error, status, task, or application service
  behavior changes.
- No Qt startup, dialog interaction, screenshots, interactive visual
  acceptance, clean-machine, signing, installer/update, deployment, or
  hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Message-dialog user outcome and acceptance |
| Developer 1 | fixed six-role workflow | Close/save/error policy ownership |
| Developer 2 | fixed six-role workflow | Message composition, locale, and typed decision projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Rawls the 2nd / Luna max | Bounded review returned no conclusion; no child PASS claimed |

## Changed files and modules

- `src/quillforge/presentation/message_surface.py` — owns common localized
  save-before-close, About, and recoverable error message composition.
- `src/quillforge/presentation/main_window.py` — delegates message composition
  and retains all close/save/error/status policy.
- `docs/adr/0049-main-window-message-surface.md` — D24 decision, invariants,
  scope, and limits.
- `docs/agent-team/reviews/D24-message-surface-parent-review.md` — parent
  review, independent no-conclusion record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D24 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- The surface exposes `ask_save_before_close()`, `show_about()`, and
  `show_error()`; it does not decide what a returned close decision does.
- MainWindow keeps the startup/close guard, dirty check, Save As choice,
  asynchronous save callback, tab removal, error phase, and status sync.
- Unexpected message-box close results fail closed to `cancel`.
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
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D24 source boundary probe | PASS | MainWindow has no QMessageBox import/call; MessageSurface owns question/about/critical; close/save/error/status/locale routes remain explicit. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D24 candidate root/dist SHA `FFAE5D29F4C6C2540EC10C66EE86C54A8B249C9E260C63467836E670571D435E`, size `38,390,152` bytes; source `tree-sha256:8b6221c3d30f518b1f975afb3994ca96a1105cc4532bf22511f8f7f99af27f46`. |
| `scripts/verify_handoff.ps1` | PASS | D24 handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 78 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

Rawls the 2nd / Luna max was assigned a bounded read-only D24 review with no
write access, no Qt launch, and no test creation/run. Two bounded wait windows
returned no conclusion; the reviewer was then closed. No child PASS or FAIL is
claimed. D24 is accepted only with parent static source reasoning and
deterministic checks.

## Simplification assessment

The extraction removes three direct QMessageBox composition paths and their
locale plumbing from MainWindow, and replaces raw button-flag handling at the
coordinator with one typed decision seam. It preserves all existing policy and
consequences; no further safe behavior-preserving simplification is required
for this slice.

## Unrun checks and reason

- QApplication/Qt startup, modal button delivery, focus, screenshots, visual
  hierarchy, screen-reader output, DPI/font behavior, and runtime acceptance —
  prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- Independent review has no conclusion; no child PASS is represented as
  evidence.
- Runtime message lifecycle and visual/accessibility behavior remain
  unverified.
- Existing D7/D8 legal, clean-machine, signing/installer/update, and release
  gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D24-AC01`, `S53`.
- Evidence: ADR-0049, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D24 verifier/check/release-no-go evidence is refreshed; obtain a
  fresh independent review window when available.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `FFAE5D29F4C6C2540EC10C66EE86C54A8B249C9E260C63467836E670571D435E` /
  `38,390,152` bytes; source
  `tree-sha256:8b6221c3d30f518b1f975afb3994ca96a1105cc4532bf22511f8f7f99af27f46`.
- Packaging note: the portable candidate was rebuilt after D24 source edits;
  it is now historical and is not release approval. The prior D23 package
  identity is also historical.

## Disposition

`accepted-with-limits`: D24 message composition is source-level verified by
the parent with an explicit independent no-conclusion record; runtime,
clean-machine, and external release gates remain open.
