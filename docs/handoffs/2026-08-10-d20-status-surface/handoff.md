# Handoff: 2026-08-10-d20-status-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d20-status-surface` |
| Delivery / slice | `D20 / ARCH-11 MainWindow status surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T15:00:00+08:00` |

## User outcome

The shell status rail now has a focused presentation owner. Ready, working,
attention, error, locale, and native status-bar placement remain unchanged;
MainWindow continues to decide when each state applies.

## Scope and boundaries

### In scope

- `StatusSurface` rail composition, widget exposure, locale, and phase methods.
- MainWindow status phase precedence, operation/document/error policy, and
  status-bar insertion through the semantic surface.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No phase taxonomy, status copy, QSS/theme, notification, TaskRunner, dirty
  state, operation, or close-policy changes.
- No new status state model, event bus, singleton, service locator, or UI
  framework.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Status visibility and shell acceptance |
| Developer 1 | fixed six-role workflow | Operation/document phase policy |
| Developer 2 | fixed six-role workflow | StatusRail presentation composition |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Carson / Luna max | Read-only D20 review; bounded result recorded below |

## Changed files and modules

- `src/quillforge/presentation/status_surface.py` — owns status rail
  composition and semantic projection.
- `src/quillforge/presentation/main_window.py` — composes StatusSurface and
  retains phase policy.
- `docs/adr/0045-main-window-status-surface.md` — D20 decision and invariants.
- `docs/agent-team/reviews/D20-status-surface-parent-review.md` — parent
  review, independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D20 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `StatusSurface` exposes only `widget`, `set_locale()`, and `set_phase()`;
  it does not expose the concrete rail to MainWindow policy.
- MainWindow retains working-over-attention precedence and explicit error
  projection, including close/operation guards.
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
| D20 StatusSurface boundary/phase/locale probe | PASS | Final static boundary/phase/locale probe passed; MainWindow retains phase policy. |
| `scripts/verify_handoff.ps1` | PASS | D20 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D20 portable candidate rebuilt with root/dist identity `F9FB9D0DB12F03B2A040FBEA05D4BFE913BCDCB3A3AFCAF620B09B6E79542233` / `38,381,994` bytes; D21 is the current candidate. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and external release gates remain open. |

## Independent review

Carson / Luna max was assigned a bounded read-only D20 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The review
confirmed status-bar parent/lifetime, locale preservation, phase precedence,
error/close policy, and no unsafe simplification. Returning the concrete rail
type from `widget` is recorded as an optional narrowing improvement only. Qt
runtime rendering remains unverified.

## Simplification assessment

The extraction removes StatusRail construction, status-bar object-tree detail,
locale projection, and direct phase receivers from MainWindow. Three thin
semantic methods preserve the existing rail without adding state. No further
safe behavior-preserving simplification is required for this bounded slice.

## Unrun checks and reason

- QApplication/Qt startup, status-bar rendering, QSS refresh, screenshots,
  screen-reader output, native metrics, DPI/font behavior, and cross-machine
  appearance — prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices should keep contracts explicit.
- Runtime status rendering, accessibility, native metrics, fonts, DPI, and
  cross-machine appearance remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D20-AC01`, `S49`.
- Evidence: ADR-0045, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D20 package and release-no-go evidence are refreshed; keep runtime
  and external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe` (historical D20 capture).
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `F9FB9D0DB12F03B2A040FBEA05D4BFE913BCDCB3A3AFCAF620B09B6E79542233` / `38,381,994` bytes; source `tree-sha256:e16dc630e3441170134212554865f6189ad947714fae84403583a471139b7412`.
- Packaging note: D20 capture is historical after D21 source edits; it is not
  release approval.

## Disposition

`accepted-with-limits`: D20 StatusSurface extraction is source-level
behavior-preserving by reviewed phase, locale, and lifecycle invariants;
runtime rendering is unrun, and the remaining architecture/release gates stay
open.
