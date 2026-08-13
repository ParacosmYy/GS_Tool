# Handoff: 2026-08-10-d18-settings-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d18-settings-surface` |
| Delivery / slice | `D18 / ARCH-09 MainWindow settings surface coordinator` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T13:00:00+08:00` |

## User outcome

Settings editing now has a focused presentation owner. The existing language,
theme, accent, font, size, motion, wrapping, and line-number controls continue
to return one validated settings snapshot, while persistence and application
policy remain in MainWindow.

## Scope and boundaries

### In scope

- `SettingsSurface` modal dialog composition and `edit()` semantic boundary.
- Existing `SettingsDialog` Save/cancel behavior and `SettingsSnapshot` return.
- MainWindow delegation while preserving settings service, worker, theme,
  locale, editor, animation, notification, and error policy.
- ADR, architecture/spec/roadmap/task, acceptance/register/index, review,
  package provenance, and release no-go synchronization.

### Out of scope

- No settings schema, persistence adapter, TaskRunner policy, theme engine,
  editor application, or error taxonomy moved into the surface.
- No new preferences, controls, locale, font, theme, or animation behavior.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Settings outcome and cancel/save acceptance |
| Developer 1 | fixed six-role workflow | SettingsService/TaskRunner/theme/editor ownership |
| Developer 2 | fixed six-role workflow | SettingsDialog presentation composition |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Peirce / Luna max | Read-only D18 review; bounded result recorded below |

## Changed files and modules

- `src/quillforge/presentation/settings_surface.py` — owns settings-dialog
  composition and the modal snapshot-return boundary.
- `src/quillforge/presentation/main_window.py` — composes SettingsSurface and
  retains settings persistence/application behavior.
- `docs/adr/0043-main-window-settings-surface.md` — D18 decision and
  invariants.
- `docs/agent-team/reviews/D18-settings-surface-parent-review.md` — parent
  review, independent-review record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projection.
- `tasks/plan.md`, `tasks/todo.md` — D18 tracking.
- `docs/agent-team/acceptance.json`, `delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `SettingsSurface.edit()` returns `SettingsSnapshot | None`; it does not
  expose the concrete dialog or application services.
- Save/cancel semantics stay in the existing dialog; MainWindow remains the
  only owner of persistence, asynchronous operation state, theme/editor
  projection, notifications, and errors.
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
| D18 SettingsSurface boundary/return-semantics probe | PASS by source reasoning; final exact probe recorded below | MainWindow delegates modal settings editing without importing SettingsDialog. |
| `scripts/verify_handoff.ps1` | PASS | D18 handoff/index status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; package coverage notice remains informational. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D18 portable candidate rebuilt with root/dist identity `E49590F24B192D39C57237B6FF9659CEE4E96913DF54057721273449307AC4EF` / `38,382,078` bytes. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and external release gates remain open. |

## Independent review

Peirce / Luna max was assigned a bounded read-only D18 review with no write
access, no Qt launch, and no test creation/run. The reviewer returned
**PASS (source-level, accepted-with-limits)** with no blocking FAIL. The
follow-up confirmed the `QWidget | None` parent contract, Accepted-only
snapshot return, retained MainWindow persistence/application policy, and
minimal safe delegation. Qt runtime delivery remains unverified.

## Simplification assessment

The extraction removes direct SettingsDialog construction and modal details
from MainWindow. One semantic `edit()` operation returns the existing immutable
snapshot without adding callback state or a second settings model. No further
safe behavior-preserving simplification is required for this bounded slice.

## Unrun checks and reason

- QApplication/Qt startup, modal signal delivery, keyboard/focus behavior,
  visual settings acceptance, screenshots, screen-reader output, native
  metrics, and DPI/font behavior — prohibited by the permanent no-launch
  policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- MainWindow remains a large editor/recovery/session/plugin coordinator; future
  slices should keep contracts explicit.
- Runtime settings interaction, accessibility, native metrics, fonts, DPI, and
  cross-machine appearance remain unproven.
- D7/D8 legal/clean-machine/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D18-AC01`, `S47`.
- Evidence: ADR-0043, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, and package manifest.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow coordinator slice only after
  D18 package and release-no-go evidence are refreshed; keep runtime and
  external release gates explicit.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `E49590F24B192D39C57237B6FF9659CEE4E96913DF54057721273449307AC4EF` / `38,382,078` bytes; source `tree-sha256:8474710bea729bea07425cca4384bf7f6931c03dbf4fd635947eaaf1e44559f2`.
- Packaging note: historical portable candidate was rebuilt after D18 source edits;
  it is not release approval.

## Disposition

`accepted-with-limits`: D18 SettingsSurface extraction is source-level
behavior-preserving by reviewed modal and snapshot invariants; runtime
interaction is unrun, and the remaining architecture/release gates stay open.
