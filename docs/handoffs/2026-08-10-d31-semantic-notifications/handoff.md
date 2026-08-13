# Handoff: 2026-08-10-d31-semantic-notifications

| Field | Value |
|---|---|
| ID | `2026-08-10-d31-semantic-notifications` |
| Delivery / slice | `D31 / UI-17 / ARCH-21 semantic transient notification hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T21:00:00+08:00` |

## User outcome

Transient shell messages now have a readable visual hierarchy: ordinary
information, successful outcomes, warnings, and errors are projected through
one localized status-message control with theme-aware highlighting. The
reported “highlights do not distinguish states” gap is addressed at the
existing presentation boundary without changing file, document, plugin, or
operation policy.

## Scope and boundaries

### In scope

- `StatusSurface` status-bar message widget, locale projection, timeout, and
  `info`/`success`/`warning`/`error` presentation levels.
- `MainWindow.notify(message, *, level="info")` compatibility seam and
  selected success/warning/error call-site metadata.
- Centralized `statusMessage` QSS selectors and all-theme/accent contrast
  evidence.
- D31 ADR, architecture/spec/roadmap/task, acceptance/register/index, parent
  review, package identity, and release no-go synchronization.

### Out of scope

- No application/domain error taxonomy rewrite, notification bus, service
  locator, parser that infers severity from text, or MainWindow rewrite.
- No command callbacks, shortcuts, workspace containment, file-first-click /
  folder-double-click behavior, plugin capability, worker lifecycle, or
  status-phase precedence changes.
- No Qt launch, screenshot, interactive visual acceptance, screen-reader,
  clean-machine, cross-machine, deployment, or hardware evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `architect` | Integration, final review, verification, and handoff decision |
| Project Manager | `fixed six-role workflow` | Plan, dependencies, risks, and release gates |
| Product | `fixed six-role workflow` | Readable, modern, visibly differentiated shell feedback |
| Developer 1 | `fixed six-role workflow` | Existing application/operation and plugin notification compatibility |
| Developer 2 | `fixed six-role workflow` | StatusSurface/QSS presentation implementation |
| QA | `Ramanujan the 2nd / Luna max + parent` | Read-only lifecycle/API, contrast, static, package, and unrun evidence |
| Architecture reviewer | `Socrates the 2nd / Luna max` | Bounded read-only architecture recommendation; no source writes |
| Independent reviewer | `Ramanujan the 2nd / Luna max` | Narrow lifecycle/API source review; PASS WITH LIMITS |

## Changed files and modules

- `src/quillforge/presentation/status_surface.py` — owns the styled transient
  message label, level, locale refresh, re-attach, and timer projection.
- `src/quillforge/presentation/main_window.py` — keeps the existing notify
  call shape and marks selected outcome/error notifications with presentation
  levels only.
- `src/quillforge/presentation/theme.py` — centralizes four status-message
  selectors using existing theme tokens; success/error message text uses the
  readable `text_primary` token on semantic backgrounds.
- `docs/adr/0056-semantic-transient-notification-hierarchy.md` — decision,
  invariants, alternatives, limits, and verification target.
- `docs/agent-team/reviews/D31-semantic-notification-parent-review.md` — parent
  review, independent review, simplification assessment, and evidence matrix.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`,
  `tasks/todo.md` — architecture and delivery projections.
- `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `docs/handoffs/index.json` —
  acceptance and handoff traceability.

## Decisions and constraints

- `StatusSurface` remains the sole transient-notification presentation
  boundary; MainWindow retains text, operation/error policy, phase precedence,
  and close guards.
- `notify(message)` remains valid as a one-positional-argument callback for
  plugins; `level` is optional presentation metadata defaulting to `info`.
- `StatusPhase.error` remains independent from notification level; no message
  parser or second lifecycle model is introduced.
- Shared checkout writer: `architect` only; child agents were read-only and no
  worktree was created or used.
- Runtime launch policy: QuillForge.exe, QApplication, screenshots, and
  interactive visual acceptance were not authorized and were not run.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Source lint/import gate. |
| `uv run ruff format --check src\quillforge` | PASS | 79 source files formatted. |
| D31 status contract probe | PASS | Typed levels, explicit widget, locale route, no native `showMessage`, and four selectors. |
| D31 notification contrast probe | PASS | 48 theme/accent message pairs >= 4.5:1. |
| JSON synchronization probe | PASS | Acceptance, register, and handoff index parse. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Rebuilt portable candidate; root/dist identity below. |
| Ramanujan / Luna independent review | PASS WITH LIMITS | Lifecycle/API/token scope source review; Qt runtime not run. |
| `scripts\verify_handoff.ps1` | PASS | Indexed handoff, status, and required sections are synchronized. |
| `scripts\check.ps1` | PASS | NOTICE, handoff, 81 formatted files, source, and package checks pass. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; 10 open gates; exact mechanical failures are `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`. |

## Unrun checks and reason

- Qt startup, status-bar rendering, timer delivery, screenshot, keyboard/focus,
  native style, installed-font metrics, DPI, screen-reader and cross-machine
  appearance — no-launch policy and unavailable authorized runtime evidence.
- Unit tests, mocks, fixtures, harnesses, or test-only assets — prohibited by
  the project verification policy for this delivery.
- D7.3 permission/disk-pressure and authorized workload evidence — requires
  operator authorization and an approved environment.
- D7.4 packaged interactive/cross-machine evidence — requires operator
  authorization and fresh runtime/report capture.
- D8 legal, signing, installer/update, clean-machine, support, and
  release-owner evidence — external approvals and environments remain open.

## Known risks and limits

- Runtime QSS specificity, QStatusBar layout allocation, timer delivery,
  accessibility output, DPI, and font availability remain unverified.
- The broader independent review window returned no conclusion; the narrower
  Ramanujan review returned PASS WITH LIMITS and does not substitute for Qt
  visual acceptance.
- Release verifier remains an explicit NO-GO with the known report-consistency
  mechanical failures and open D7/D8 gates.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or release-readiness claim is made.

## Acceptance and evidence IDs

- Acceptance: `D31-AC01`, `S60`
- Evidence: `docs/adr/0056-semantic-transient-notification-hierarchy.md`,
  `docs/agent-team/reviews/D31-semantic-notification-parent-review.md`,
  `D31 status contract probe PASS`,
  `D31 notification contrast probe PASS: 48 pairs >= 4.5:1`

## Next owner and next action

- Owner: `architect / release owner when authorized`
- Action: run the handoff/check/release-no-go verifiers, then obtain
  authorized runtime visual/accessibility and D7/D8 release evidence; keep the
  next code slice bounded to a distinct user-visible gap.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `E4DAFAA9DE4C1BE65E380989E781F1706E6BDA9EFCB4BADE686BDA37A5419397` / `38,403,936` bytes (root/dist identical)
- Source revision: `tree-sha256:01d9be56d8283a2e27277e05f0ddc7b308b6057856b6a07e144dbb1796d1ec65`
- Packaging note: portable one-file candidate rebuilt; unsigned, no installer,
  no updater, and no file associations remain explicit release decisions.

## Disposition

`accepted-with-limits`: D31 source behavior, architecture boundary, contrast
probe, independent source review, simplification assessment, and package
identity are recorded. The next handoff remains conditional on authorized
runtime/accessibility and external D7/D8/release evidence.
