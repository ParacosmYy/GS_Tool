# Handoff: 2026-08-10-d19-recovery-prompt-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d19-recovery-prompt-surface` |
| Delivery / slice | `D19 / ARCH-10 MainWindow recovery prompt surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T14:00:00+08:00` |

## User outcome

Recovery prompts now have a focused presentation owner. The existing restore,
discard, and later choices, source-status explanation, timestamp, untitled
fallback, and locale behavior remain intact while recovery/session policy stays
in MainWindow.

## Scope and boundaries

### In scope

- `RecoveryPromptSurface` prompt parentage, locale, text projection, button
  roles, and typed `restore/discard/later` decision.
- MainWindow mapping from `RecoveryCandidate` to semantic prompt inputs and
  retention of all recovery service/session/document/notification branches.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No RecoveryService calls, snapshot mutation, deletion scheduling, session
  deferred-path state, document event publication, or recovery error policy
  moved into the surface.
- No new recovery behavior, storage format, session behavior, or prompt copy.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Recovery choice and safety acceptance |
| Developer 1 | fixed six-role workflow | Recovery/session/document policy ownership |
| Developer 2 | fixed six-role workflow | Recovery prompt presentation composition |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Linnaeus / Luna max | Read-only D19 review; bounded result recorded below |

## Changed files and modules

- `src/quillforge/presentation/recovery_prompt_surface.py` — owns recovery
  prompt composition and typed decision projection.
- `src/quillforge/presentation/main_window.py` — delegates prompt presentation
  and retains recovery/session/document policy.
- `docs/adr/0044-main-window-recovery-prompt-surface.md` — D19 decision and
  invariants.
- `docs/agent-team/reviews/D19-recovery-prompt-surface-parent-review.md` —
  parent review, independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D19 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `RecoveryPromptSurface.choose()` returns only the typed semantic decision;
  it does not expose `QMessageBox`, RecoveryService, or mutable recovery state.
- MainWindow remains the only owner of restore/discard/defer consequences,
  session continuity, document events, cleanup, and notifications.
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
| `uv run python -m compileall -q src\quillforge` | PASS | Compile gate is non-destructive. |
| `uv run ruff check src\quillforge` | PASS | Lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D19 recovery-prompt decision/text/ownership probe | PASS | Final static boundary/decision probe passed; MainWindow consumes the typed decision and retains policy. |
| `scripts/verify_handoff.ps1` | PASS | D19 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D19 portable candidate rebuilt with root/dist identity `826BD6F2D3CDCB25CCF488908A37ABABE1720C8C3D2ABA8E44204B39A963D800` / `38,382,128` bytes. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and external release gates remain open. |

## Independent review

Linnaeus / Luna max was assigned a bounded read-only D19 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed three-state mapping, dismissal-as-later, retained MainWindow recovery
branches, text/locale invariants, and no unsafe simplification. A non-blocking
concern is recorded: `source_status` remains a defensive `str` boundary rather
than a closed type; existing diagnostic-runner layering is explicitly outside
D19 scope. Qt runtime delivery remains unverified.

## Simplification assessment

The extraction removes recovery prompt construction, translation mapping,
timestamp formatting, and button identity details from MainWindow. One typed
decision surface replaces those details without duplicating recovery state or
policy. No further safe behavior-preserving simplification is required for
this bounded slice.

## Unrun checks and reason

- QApplication/Qt startup, modal button delivery, locale rendering,
  focus/keyboard behavior, screenshots, screen-reader output, native metrics,
  DPI/font behavior, and recovery timing — prohibited by the permanent
  no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices should keep contracts explicit.
- Runtime recovery interaction, accessibility, native metrics, fonts, DPI, and
  recovery timing remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D19-AC01`, `S48`.
- Evidence: ADR-0044, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D19 package and release-no-go evidence are refreshed; keep runtime
  and external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `826BD6F2D3CDCB25CCF488908A37ABABE1720C8C3D2ABA8E44204B39A963D800` / `38,382,128` bytes; source `tree-sha256:c5522eaea358c4c520b797c08406b80df350a28e1ac9a50a94fe9d14bae2cdb8`.
- Packaging note: historical portable candidate was rebuilt after D19 source edits;
  it is not release approval.

## Disposition

`accepted-with-limits`: D19 RecoveryPromptSurface extraction is source-level
behavior-preserving by reviewed decision and ownership invariants; runtime
interaction is unrun, and the remaining architecture/release gates stay open.
