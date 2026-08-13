# Handoff: 2026-08-10-d35a-plugin-notification-severity

| Field | Value |
|---|---|
| ID | `2026-08-10-d35a-plugin-notification-severity` |
| Delivery / slice | `D35a / UI-21 / ARCH-25 Plugin and extension notification severity closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:20:00+08:00` |

## User outcome

Plugin and extension operations now read as distinct progress, success,
attention, and failure states in the shell notification surface. A failed
session manifest and a deferred recovery snapshot also use the corrected
error/warning hierarchy.

## Scope and boundaries

### In scope

- Existing `MainWindow` plugin/extension `notify` calls for catalog scans,
  catalog results, governance, runtime enablement, and isolated host probes.
- Corrected D34a classifications for invalid/failed session persistence and
  deferred recovery review.
- Reuse of the existing `StatusMessageLevel`/`StatusSurface` contract.

### Out of scope

- Plugin catalog parsing, trust/approval, runtime lifecycle, host containment,
  execution policy, TaskRunner, operation IDs, stale guards, and command-menu
  refresh behavior.
- New notification services, new state models, localized string parsing, and
  full `MainWindow` extraction.
- Qt startup, screenshots, interactive plugin/session flows, accessibility,
  clean-machine evidence, deployment, and release approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | User outcome and acceptance |
| Developer 1 | Parent architect | MainWindow call-site classification |
| Developer 2 | Parent architect | Contract/docs/package synchronization |
| QA | Parent architect | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — explicit plugin/extension
  levels and the two D34a corrections.
- `docs/adr/0060-main-window-plugin-notification-severity.md` — bounded
  architecture decision and applicability record.
- `docs/agent-team/reviews/D35a-plugin-notification-parent-review.md` — parent
  review, independent no-conclusion record, simplification assessment, and
  verification limits.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Decisions and constraints

- Unavailable/invalid/failure outcomes are `error`; in-flight/rejected/
  attention outcomes are `warning`; in-progress transient messages are
  explicit `info` while the permanent TaskRunner/status phase is `WORKING`;
  clean completion is `success`, with typed catalog diagnostics retaining
  `warning`.
- MainWindow remains outcome/policy owner and StatusSurface remains presentation
  owner. Notification text is never parsed to infer severity.
- Fermat the 2nd / Luna max architecture consultation returned no conclusion;
  Cicero the 2nd / Luna max independent review also returned no conclusion.
  No child PASS is claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge/presentation/main_window.py` | PASS | Changed coordinator source compiled. |
| `uv run ruff check src/quillforge/presentation/main_window.py` | PASS | No diagnostics. |
| `uv run ruff format --check src/quillforge/presentation/main_window.py` | PASS | File is formatted. |
| D35a plugin-level source probe | PASS | All targeted plugin/extension notification calls have legal explicit levels. |
| D34a correction source probe | PASS | Invalid/failed persistence is error; deferred recovery is warning. |
| `scripts/package.ps1` | PASS | Synchronized portable artifact; identity recorded below. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register structure synchronized. |
| `scripts/check.ps1` | PASS | Repository static and evidence checks pass. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and release-owner gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, status rendering, plugin dialogs, host diagnostics,
  session restore, screenshots, screen-reader output, font metrics, DPI, and
  cross-machine appearance — blocked by the active no-launch policy and lack of
  authorized runtime acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Runtime QSS specificity, actual color/font/DPI geometry, accessibility, and
  screen-reader presentation remain user-owned acceptance items.
- Remaining document/workspace notification calls are not classified by this
  slice; future work must continue to use explicit levels rather than parsing
  message text.
- The package is an unsigned portable candidate, not an enterprise release;
  clean-machine, signing, installer/update, and release-owner decisions remain
  open.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow document/workspace notification
  audit or typed coordinator contract slice; preserve explicit levels and the
  current service/policy ownership boundaries.

## Acceptance and evidence IDs

- Acceptance: `D35A-AC01`, `S64`.
- Evidence: `docs/adr/0060-main-window-plugin-notification-severity.md`,
  `docs/agent-team/reviews/D35a-plugin-notification-parent-review.md`,
  `D35a-plugin-level-contract=PASS`, `D34a-correction=PASS`,
  `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and
  `dist/QuillForge.release.json`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identical in the current candidate.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the D35a source slice is integrated with explicit
architecture, independent-review status, simplification, and static
verification evidence. Runtime visual acceptance, remaining coordinator
notification calls, and release-owner gates remain conditions for later work.
