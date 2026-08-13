# Handoff: 2026-08-10-d26-status-surface-host

| Field | Value |
|---|---|
| ID | `2026-08-10-d26-status-surface-host` |
| Delivery / slice | `D26 / ARCH-17 status surface host projection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T18:00:00+08:00` |

## User outcome

The status rail and transient notifications now share one status presentation
owner. Locale changes continue to update both, while MainWindow remains the
owner of status phase policy, worker/document state, and notification call-site
decisions.

## Scope and boundaries

### In scope

- `StatusSurface` status-bar host attachment, size-grip configuration,
  permanent rail projection, locale state, and localized transient messages.
- MainWindow delegation while retaining phase precedence, TaskRunner/document
  policy, and notification call sites.
- D26 ADR, review, acceptance/register/index, architecture/spec/roadmap/task,
  package provenance, and release no-go synchronization.

### Out of scope

- No status phase precedence, worker lifecycle, document policy, message text,
  timeout default, or application behavior changes.
- No Qt startup, status-bar interaction, screenshots, interactive visual
  acceptance, clean-machine, signing, installer/update, deployment, or
  hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Architecture decision, implementation, integration, verification, handoff |
| Project Manager | fixed six-role workflow | Dependencies, risks, and open release gates |
| Product | fixed six-role workflow | Status/notification user outcome and acceptance |
| Developer 1 | fixed six-role workflow | Phase/runner/document policy ownership |
| Developer 2 | fixed six-role workflow | Status-bar host, locale, and notification projection |
| QA | fixed six-role workflow | Static/package/no-launch verification |
| Independent reviewer | Leibniz the 2nd / Luna max | Bounded review returned no conclusion; no child PASS claimed |

## Changed files and modules

- `src/quillforge/presentation/status_surface.py` — owns status-bar host
  attachment, size-grip choice, rail projection, locale, and notifications.
- `src/quillforge/presentation/main_window.py` — attaches the surface and
  delegates notifications while retaining phase/policy ownership.
- `docs/adr/0051-status-surface-host-projection.md` — D26 decision,
  invariants, scope, and limits.
- `docs/agent-team/reviews/D26-status-surface-host-parent-review.md` — parent
  review, independent no-conclusion record, simplification, and validation.
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md` — architecture and migration projections.
- `tasks/plan.md`, `tasks/todo.md` — D26 tracking.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json` — acceptance and delivery projections.

## Decisions and constraints

- `StatusSurface.attach_to()` owns `QStatusBar` configuration and the
  permanent `StatusRail` placement; repeated attachment to the same host is a
  no-op.
- `StatusSurface.show_message()` uses its normalized locale and the existing
  5000 ms timeout default; it safely no-ops before a host is attached.
- MainWindow keeps phase precedence, task/document policy, and all notification
  call sites; the surface owns only presentation projection.
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
| `uv run ruff format --check src\quillforge` | PASS | Format gate; 77 source files were already formatted. |
| D26 source boundary probe | PASS | MainWindow has no direct status-bar configuration/message calls; StatusSurface owns host/localization while phase policy remains in MainWindow. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D26 candidate root/dist SHA `34337021DF82E9144F97F68CEC79292DD08749DBC406D6BECA4F54F074B62ED8`, size `38,392,052` bytes; source `tree-sha256:ed91c1474dfb6b9dadc6ed1cfb606bd14f6315096b08d7151f6e97c26a06a831`. |
| `scripts/verify_handoff.ps1` | PASS | D26 handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 79 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

## Independent review

Leibniz the 2nd / Luna max was assigned a bounded read-only D26 review with no
write access, no Qt launch, and no test creation/run. Two bounded wait windows
returned no conclusion; the reviewer was then closed. No child PASS or FAIL is
claimed. D26 is accepted only with parent static source reasoning and
deterministic checks.

## Simplification assessment

The extraction removes direct status-bar configuration and notification
localization from MainWindow while preserving all phase and operation policy.
The API stays limited to host attachment, locale, transient message, phase,
and existing rail projection; no further safe behavior-preserving
simplification is required for this slice.

## Unrun checks and reason

- QApplication/Qt startup, status-bar rendering, notification timing, focus,
  screenshots, visual hierarchy, screen-reader output, DPI/font behavior, and
  runtime acceptance — prohibited by the permanent no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, cross-machine, signing, installer/update, deployment, and
  hardware checks — not authorized.

## Known risks and limits

- Independent review has no conclusion; no child PASS is represented as
  evidence.
- Runtime status-bar attachment, notification timing, visual/accessibility
  behavior, and cross-machine rendering remain unverified.
- Existing D7/D8 legal, clean-machine, signing/installer/update, and release
  gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D26-AC01`, `S55`.
- Evidence: ADR-0051, source modules, architecture/spec/roadmap/task records,
  parent review, this handoff, static checks, handoff verifier, package
  manifest, and release no-go dossier.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application coordinator slice
  only after D26 verifier/check/release-no-go evidence is refreshed; obtain a
  fresh independent review window when available.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `34337021DF82E9144F97F68CEC79292DD08749DBC406D6BECA4F54F074B62ED8` /
  `38,392,052` bytes; source
  `tree-sha256:ed91c1474dfb6b9dadc6ed1cfb606bd14f6315096b08d7151f6e97c26a06a831`.
- Packaging note: D26's portable candidate was rebuilt after D26 source edits;
  it is not the current release candidate and is retained as historical
  evidence. D25 and D26 package identities are historical.

## Disposition

`accepted-with-limits`: D26 status host composition is source-level verified by
the parent with an explicit independent no-conclusion record; runtime,
clean-machine, and external release gates remain open.
