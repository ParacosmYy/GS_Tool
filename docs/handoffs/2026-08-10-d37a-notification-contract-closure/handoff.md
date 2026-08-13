# Handoff: 2026-08-10-d37a-notification-contract-closure

| Field | Value |
|---|---|
| ID | `2026-08-10-d37a-notification-contract-closure` |
| Delivery / slice | `D37a / UI-23 / ARCH-27 MainWindow notification contract closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

The shell now gives every current MainWindow notification an explicit
info/success/warning/error meaning. New-document success, session fallback,
stale-command, settings guard, and active-tab feedback no longer depend on an
implicit default. Long-running work keeps the permanent `WORKING` status rail
and uses an informational transient message.

## Scope and boundaries

### In scope

- The six remaining MainWindow notification call-site classifications.
- The explicit-level contract across all 81 unique MainWindow `notify` calls.
- Preservation of D34a–D36a Session/Recovery, plugin, workspace, and operation
  mappings.
- Existing `StatusMessageLevel`/`StatusSurface` reuse and one-argument API
  compatibility.

### Out of scope

- New notification services, message parsing, state machines, or full
  MainWindow extraction.
- Document/session/settings/plugin/workspace policy, operation IDs, TaskRunner,
  close guards, locale routing, and service ownership changes.
- Qt startup, screenshots, interactive flows, accessibility, clean-machine
  evidence, deployment, signing, and release approval.

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

- `src/quillforge/presentation/main_window.py` — explicit metadata for the
  remaining document/session/settings/command/tab notifications and D36a
  contract corrections.
- `docs/adr/0062-main-window-notification-contract-closure.md` — bounded
  architecture decision and public-source applicability record.
- `docs/agent-team/reviews/D37a-main-window-notification-parent-review.md` —
  parent review, independent review status, simplification assessment, and
  verification limits.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture and
  acceptance projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/RELEASE_HANDOFF.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability and package evidence.

## Decisions and constraints

- The transient shell contract is exactly `info`/`success`/`warning`/`error`.
  In-progress operations keep the permanent status phase at `WORKING` and
  project their transient copy as `info`.
- MainWindow remains outcome/policy owner and StatusSurface remains
  presentation owner. Notification text is never parsed to infer severity.
- Wegener the 2nd / Luna max architecture consultation returned no
  conclusion; no architecture PASS is claimed. Curie the 2nd / Luna max
  independent review returned **PASS with limits** for the source contract,
  branch mappings, ownership, safety, and performance. Mill the 2nd / Luna
  max follow-up timed out twice and was closed without a conclusion; it is not
  counted as a PASS.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch policy is not allowed; static, compilation, packaging, and
  source evidence only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| MainWindow AST notification-contract probe | PASS | 81 unique calls; explicit legal levels; zero transient `working` levels. |
| D36a `limit_reason` source probe | PASS | Normal `"none"` is not treated as a warning. |
| Restore-guard source probe | PASS | Eleven matching startup restore notifications are explicit. |
| `uv run python -m compileall -q src/quillforge/presentation/main_window.py` | PASS | Changed coordinator source compiled. |
| `uv run ruff check src/quillforge/presentation/main_window.py` | PASS | No diagnostics; non-fatal cache warning may be emitted. |
| `uv run ruff format --check src/quillforge/presentation/main_window.py` | PASS | File is formatted. |
| `scripts\verify_handoff.ps1` | PASS | Handoff/index/register structure synchronized. |
| `scripts\check.ps1` | PASS | Repository static, compile, format, and evidence checks pass. |
| `scripts\package.ps1` | PASS | Root/dist portable candidates rebuilt with matching identity below. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Ten release gates remain open; three historical report-binding failures are recorded. |

## Unrun checks and reason

- QApplication/Qt startup, status rendering, document/session/settings flows,
  screenshots, screen-reader output, font metrics, DPI, and cross-machine
  appearance — blocked by the active no-launch policy and lack of authorized
  runtime acceptance.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Runtime QSS specificity, actual color/font/DPI geometry, accessibility, and
  screen-reader presentation remain user-owned acceptance items.
- The package is an unsigned portable candidate, not an enterprise release;
  clean-machine, signing, installer/update, and release-owner decisions remain
  open.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Next owner and next action

- Owner: Architect.
- Action: continue a separate bounded MainWindow coordinator decomposition and
  observability audit; preserve the explicit four-level transient contract and
  permanent WORKING phase boundary.

## Acceptance and evidence IDs

- Acceptance: `D37A-AC01`, `S66`.
- Evidence: `docs/adr/0062-main-window-notification-contract-closure.md`,
  `docs/agent-team/reviews/D37a-main-window-notification-parent-review.md`,
  `D37a-notification-contract=PASS`, `D36a-search-limit-severity=PASS`,
  `D36a-restore-guard=PASS`, `scripts\verify_handoff.ps1`,
  `scripts\check.ps1`, and `dist\QuillForge.release.json`.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identical.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the D37a source slice is integrated with explicit
architecture, independent-review status, simplification, and static
verification evidence. Runtime visual acceptance, deeper coordinator
extraction, and release-owner gates remain conditions for later work.
