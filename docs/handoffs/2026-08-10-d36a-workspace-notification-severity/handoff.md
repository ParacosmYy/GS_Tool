# Handoff: 2026-08-10-d36a-workspace-notification-severity

| Field | Value |
|---|---|
| ID | `2026-08-10-d36a-workspace-notification-severity` |
| Delivery / slice | `D36a / UI-22 / ARCH-26 Workspace and operation notification severity closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:45:00+08:00` |

## User outcome

Workspace navigation, Find in Files, file-boundary rejection, cancellation,
and long-running operations now show distinct progress, success, attention,
and failure states in the shell notification channel. The existing inline
workspace/search feedback remains available for detailed context.

## Scope and boundaries

### In scope

- Workspace selection/search availability, root validation, startup restore
  guards, containment rejections, search completion/failure, workspace open/
  directory completion/failure, and cancellation notifications.
- `_begin_operation` progress projection for existing open/save/workspace/
  Replace All operations.
- Existing `StatusMessageLevel`/`StatusSurface` reuse and one invalid-search
  result shell error projection.

### Out of scope

- Workspace/search services, TaskRunner, generation/operation IDs, cancellation
  tokens, root containment, session restore barriers, document opening, and
  close/error policy.
- New notification services, new state models, localized string parsing, full
  MainWindow decomposition, settings and unrelated document/session calls.
- Qt startup, screenshots, interactive workspace/search/file flows,
  accessibility, clean-machine evidence, deployment, and release approval.

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

- `src/quillforge/presentation/main_window.py` — explicit Workspace/search/
  operation levels and invalid-result shell projection.
- `docs/adr/0061-workspace-operation-notification-severity.md` — bounded
  architecture decision and applicability record.
- `docs/agent-team/reviews/D36a-workspace-notification-parent-review.md` —
  parent review, independent no-conclusion record, simplification assessment,
  and verification limits.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability and package evidence.

## Decisions and constraints

- Existing long-running operation progress is projected by the permanent
  status phase as `WORKING`; its transient copy is explicit `info`. Typed
  workspace/search outcomes use `success`/`warning`/`error` without parsing
  copy.
- MainWindow remains outcome/policy owner; WorkspaceSurface,
  WorkspaceSearchSurface, and StatusSurface remain presentation owners.
- Noether the 2nd / Luna max architecture consultation returned no conclusion;
  Nietzsche the 2nd / Luna max independent review also returned no conclusion.
  No child PASS is claimed.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge/presentation/main_window.py` | PASS | Changed coordinator source compiled. |
| `uv run ruff check src/quillforge/presentation/main_window.py` | PASS | No diagnostics; one non-fatal cache access warning was emitted. |
| `uv run ruff format --check src/quillforge/presentation/main_window.py` | PASS | File is formatted. |
| D36a workspace-level source probe | PASS | Targeted Workspace/search notifications have legal explicit levels. |
| D36a restore-guard source probe | PASS | All eleven restore guard notifications have explicit levels. |
| `scripts/package.ps1` | PASS | Synchronized portable artifact; identity recorded below. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register structure synchronized. |
| `scripts/check.ps1` | PASS | Repository static and evidence checks pass. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Runtime/report freshness and release-owner gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, status rendering, workspace dock, Find in Files,
  file activation, screenshots, screen-reader output, font metrics, DPI, and
  cross-machine appearance — blocked by the active no-launch policy and lack
  of authorized runtime acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Runtime QSS specificity, actual color/font/DPI geometry, and accessibility
  remain user-owned acceptance items.
- Remaining settings/document/session notifications are not classified by
  this slice; future work must continue to use explicit levels.
- The package is an unsigned portable candidate, not an enterprise release;
  clean-machine, signing, installer/update, and release-owner decisions remain
  open.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow settings/document/session
  notification or typed coordinator contract slice; preserve explicit levels
  and current service/policy ownership.

## Acceptance and evidence IDs

- Acceptance: `D36A-AC01`, `S65`.
- Evidence: `docs/adr/0061-workspace-operation-notification-severity.md`,
  `docs/agent-team/reviews/D36a-workspace-notification-parent-review.md`,
  `D36a-workspace-level-contract=PASS`, `D36a-restore-guard=PASS`,
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

`accepted-with-limits`: the D36a source slice is integrated with explicit
architecture, independent-review status, simplification, and static
verification evidence. Runtime visual acceptance, remaining coordinator
notification calls, and release-owner gates remain conditions for later work.
