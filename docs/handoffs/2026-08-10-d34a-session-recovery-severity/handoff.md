# Handoff: 2026-08-10-d34a-session-recovery-severity

| Field | Value |
|---|---|
| ID | `2026-08-10-d34a-session-recovery-severity` |
| Delivery / slice | `D34a / UI-20 / ARCH-24 Session/Recovery notification severity closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T22:30:00+08:00` |

## User outcome

Session and Recovery success, neutral information, user-attention fallback,
and failure outcomes now receive distinct visual severity through the existing
status-message hierarchy, so recovery problems are no longer presented as
ordinary informational text.

## Scope and boundaries

### In scope

- Explicit legal severity on 19 existing Session/Recovery `notify` calls.
- Reuse of the existing StatusMessageLevel/StatusSurface contract.
- Static policy-boundary and compatibility evidence.

### Out of scope

- SessionService, RecoveryService, TaskRunner, snapshot lifecycle, persistence
  payloads, operation IDs, stale guards, close guards, and callback order.
- Plugin/document/workspace notification call sites outside Session/Recovery.
- Qt startup, screenshots, interactive recovery/session flows, accessibility,
  clean-machine, deployment, and enterprise release approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, final review, verification, and handoff decision |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and status |
| Product | User / product owner | User outcome and acceptance |
| Developer 1 | Parent architect | MainWindow severity call-site mapping |
| Developer 2 | Parent architect | Status contract/package synchronization |
| QA | Parent architect | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/main_window.py` — explicit levels for the
  Session/Recovery notification outcomes only.
- `docs/adr/0059-session-recovery-notification-severity.md` — architecture
  decision and verification target.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — boundary and acceptance
  records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  and `docs/handoffs/index.json` — traceability records.

## Decisions and constraints

- Severity is explicit metadata; notification text is never parsed.
- MainWindow remains outcome/policy owner; StatusSurface remains presentation
  owner; no recovery/session state machine was extracted.
- Herschel the 2nd / Luna max architecture consultation returned no
  conclusion; no architecture PASS is claimed. Bernoulli the 2nd / Luna max
  independent review window returned no conclusion; no child PASS is claimed.
- Shared checkout writer: parent architect; exact scope is the D34a source and
  evidence files listed in this handoff.
- Runtime launch policy: not allowed under the active no-launch policy; static
  and package validation only.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src` | PASS | Source compiled without diagnostics. |
| `uv run ruff check` on changed presentation modules | PASS | All checks passed. |
| `uv run ruff format --check` on changed presentation modules | PASS | All changed files are formatted. |
| D34a Session/Recovery notify-level probe | PASS | 19 targeted calls have legal explicit levels; policy boundary unchanged. |
| `scripts/package.ps1` | PASS | Portable artifact rebuilt; identity will be recorded below. |
| `scripts/verify_handoff.ps1` | PASS | Final synchronized handoff/register/index checks. |
| `scripts/check.ps1` | PASS | NOTICE inventory, handoff, static, and package checks pass. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Existing report freshness/startup and release-owner gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, native status rendering, screenshots, and
  interactive recovery/session flows — no-launch policy and no visual runtime
  authorization.
- Screen-reader, font metrics, DPI, clean-machine/cross-machine, remaining
  notification closure, and external release evidence — requires authorized
  QA/release-owner execution.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D execution policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- The remaining MainWindow notification calls are not all severity-closed;
  future slices must not infer severity from text.
- Runtime QSS specificity and actual font/DPI geometry remain user-owned
  acceptance items.
- The package is an unsigned portable candidate, not an enterprise release.
- Public CloudWeGo material is an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Acceptance and evidence IDs

- Acceptance: `D34A-AC01`, `S63`.
- Evidence: `docs/adr/0059-session-recovery-notification-severity.md`,
  `docs/agent-team/reviews/D34a-session-recovery-parent-review.md`, source
  probe above, `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and package
  manifest `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: Architect.
- Action: audit the next bounded MainWindow notification/coordinator domain;
  retain explicit level arguments and avoid a full-window rewrite.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `A6579E0DF3E6FFA4F4540EF12686562FB93E1AB33F718E9A50859EA6974B6EC3` / `38,415,189` bytes; root/dist identical in the current candidate.
- Source revision: `tree-sha256:168a5e6f9843edb39ee86c71e7dcb93687f099672bed06f91614539100fdc24d`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the D34a static/source slice is integrated with
explicit architecture, review, simplification, and verification evidence.
Runtime visual acceptance, remaining notification closure, and release-owner
gates remain conditions for later work.
