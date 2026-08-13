# Handoff: 2026-08-10-d23-plugin-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d23-plugin-surface` |
| Delivery / slice | `D23 / ARCH-14 MainWindow plugin surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T16:30:00+08:00` |

## User outcome

Extension Catalog and Plugin Status now share one focused presentation owner.
Dialog replacement, activation, locale refresh, semantic governance/toggle
signals, and worker-owned governance button state are centralized while
MainWindow remains the owner of all plugin operations and policy.

## Scope and boundaries

### In scope

- `PluginSurface` catalog/status dialog lifecycle, locale, activation, four
  callback routes, and governance-action projection.
- MainWindow delegation while retaining PluginCatalog/Approval/Runtime,
  TaskRunner, operation state, validation, policy, notifications, and errors.
- D23 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No plugin scanning, approval persistence, runtime enablement, trust policy,
  external execution, protocol, or service behavior changes.
- No new plugin state model, dependency, UI framework, or metadata DTO.
- No QApplication startup, screenshots, interactive visual acceptance,
  clean-machine, signing, installer/update, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Plugin dialog user outcome and acceptance |
| Developer 1 | fixed six-role workflow | Scan/approval/runtime policy ownership |
| Developer 2 | fixed six-role workflow | Dialog lifecycle, callbacks, locale, and projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Godel / Luna max | Bounded review window returned no conclusion; no child PASS claimed |

## Changed files and modules

- `src/quillforge/presentation/plugin_surface.py` — owns catalog/status dialog
  construction, replacement, activation, locale, signal routes, and
  governance-action projection.
- `src/quillforge/presentation/main_window.py` — composes PluginSurface and
  retains plugin services, async operation state, policy, and consequences.
- `docs/adr/0048-main-window-plugin-surface.md` — D23 decision, invariants,
  scope, and limits.
- `docs/agent-team/reviews/D23-plugin-surface-parent-review.md` — parent
  review, independent no-conclusion record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D23 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- Catalog and status dialogs are replaced through semantic surface methods;
  MainWindow no longer reaches into concrete dialog instances.
- Four signals remain stable: catalog approve/revoke and runtime enable/disable.
- Governance controls are enabled/disabled through the surface, but the
  inflight flag and operation policy remain in MainWindow.
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
| D23 source boundary probe | PASS | MainWindow has no concrete plugin dialog imports/construction; surface has lifecycle, locale, four callback, and governance projection routes. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D23 portable candidate; root/dist SHA `A1B68658E4FBA96923421077E214F0204A53F6EB7A3225C4D968FBDEC5C6712E`, size `38,388,423` bytes; source `tree-sha256:b23efff15976bf804ce8be01dd4aeff89a87a201d190227c9e1dad4347b61768`. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 77 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

Godel / Luna max was assigned a bounded read-only D23 review with no write
access, no Qt launch, and no test creation/run. The review window returned no
conclusion before the bounded handoff window; no child PASS or FAIL is claimed.
The parent records this exact limitation and accepts D23 only with static
source reasoning plus deterministic checks. A later independent review is
required before any release-level confidence claim.

## Simplification assessment

The extraction removes duplicate catalog/status dialog lifecycle blocks, locale
branches, signal wiring, and governance-widget reach-through from MainWindow.
It introduces no plugin execution, lifecycle state, or duplicate inflight
model. No further safe behavior-preserving simplification is required.

## Unrun checks and reason

- QApplication/Qt startup, dialog signal delivery, focus, screenshots,
  screen-reader output, DPI/font behavior, and visual acceptance — prohibited
  by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- Independent review has no conclusion; no child PASS is represented as
  evidence.
- Runtime dialog lifecycle and visual/accessibility behavior remain
  unverified.
- Existing D6 plugin trust/approval/enablement/external-execution gates remain
  in force and are not reopened or broadened by D23.
- Runtime, release, and external gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D23-AC01`, `S52`.
- Evidence: ADR-0048, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D23 verifier/check/release-no-go evidence is refreshed; obtain a
  fresh independent review window for plugin composition when available.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe` (historical D23 candidate).
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `A1B68658E4FBA96923421077E214F0204A53F6EB7A3225C4D968FBDEC5C6712E` /
  `38,388,423` bytes; source
  `tree-sha256:b23efff15976bf804ce8be01dd4aeff89a87a201d190227c9e1dad4347b61768`.
- Packaging note: the portable candidate was rebuilt after D23 source edits;
  it is now historical and is not release approval.

## Disposition

`accepted-with-limits`: D23 plugin dialog composition is source-level verified
by the parent with an explicit independent no-conclusion record; runtime,
clean-machine, and external release gates remain open.
