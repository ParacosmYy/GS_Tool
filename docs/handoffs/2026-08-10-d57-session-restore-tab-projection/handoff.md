# Handoff: 2026-08-10-d57-session-restore-tab-projection

| Field | Value |
|---|---|
| ID | `2026-08-10-d57-session-restore-tab-projection` |
| Delivery / slice | `D57 / ARCH-46 session-restore tab projection boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T16:00:00+08:00` |

## User outcome

The session-restore coordinator has a smaller enterprise boundary: ordered
restored-tab references and active-tab selection input are owned by the
framework-neutral restore tracker, while the application coordinator keeps
services, async execution, startup/close policy, and visible tab projection.

## Scope and boundaries

### In scope

- Generic opaque-tab storage in `SessionRestoreTracker`.
- Ordered record calls for existing and newly opened restore tabs.
- Canonical active-path selection with first-tab fallback.
- Begin/finish projection cleanup and static/package traceability evidence.

### Out of scope

- No Qt widget, editor, service, TaskRunner, notification, locale, theme,
  runtime event, or public application behavior redesign.
- No new tracker, path cache, file-opening change, session schema change,
  runtime launch, screenshot, clean-machine, signing, installer, updater,
  legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Erdos the 2nd / Terra max | Cross-module boundary consultation; no conclusion after two bounded waits |
| Independent review | Bohr the 2nd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/session_restore_tracker.py` — adds generic
  opaque-tab recording, active-path selection, and lifecycle cleanup.
- `src/quillforge/presentation/main_window.py` — delegates restore-tab recording
  and final target selection while retaining application policy.
- `docs/adr/0082-session-restore-tab-projection-boundary.md` — decision.
- `docs/agent-team/reviews/D57-session-restore-tab-projection-parent-review.md`
  and `D57-session-restore-tab-projection-independent-review.md` — review
  records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability.

## Decisions and constraints

- The tracker owns only restore lifecycle values and opaque tab projection;
  `path_of` prevents coupling to `_DocumentTab`, Qt, or `EditorWidget`.
- MainWindow remains the owner of session/recovery/workspace/document services,
  TaskRunner, open dispatch, tab-surface projection, notifications, startup,
  initial-document, and close policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D57 tab-projection boundary probe | `PASS` | Tracker is generic/Qt-free; MainWindow has no parallel list. |
| D57 order/active/fallback/cleanup probe | `PASS` | Three record points and begin/finish cleanup remain present. |
| Targeted compileall / Ruff / format | `PASS` | Changed source slice. |
| Full compileall / Ruff / format | `PASS` | Run after final documentation/package sync. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff indexed and synchronized. |
| `scripts\check.ps1` | `PASS` | Acceptance/register synchronized. |
| `scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing external gates and stale report bindings remain. |

## Unrun checks and reason

- Native Qt tab events, queued callback timing, runtime startup, screenshots,
  accessibility, DPI, fonts, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native event ordering or asynchronous
  restore callback timing.
- Both delegated architecture and independent review windows returned no
  conclusion; no child PASS is claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D57-AC01`, `S86`.
- Evidence: ADR-0082, source probes, parent/independent reviews, static
  checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application contract extraction
  or obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `D02C769894E567D0ABEA9851DB1316E77B40608C51509E244CC7EA4C6EF31133` / `38,435,006` bytes; root/dist identity matches.
- Source revision: `tree-sha256:5f3b855b055834f54d1d1f19d1d6da07d112b0ab620ee5f6da0cf5207474f97f`.

## Disposition

`accepted-with-limits`: the source boundary is integrated and statically
verified; package identity and full traceability are synchronized after the
final package, while runtime and release gates remain open.
