# Handoff: 2026-08-12-d308-workspace-open-affordance

| Field | Value |
|---|---|
| ID | `2026-08-12-d308-workspace-open-affordance` |
| Delivery / slice | `D308 / UI-127 / ARCH-278 Workspace open-action affordance` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:30:00+08:00` |

## User outcome

The Workspace dock now makes its two opening paths explicit: the user can
choose a project folder or choose one file directly. Both actions expose
localized hints and screen-reader descriptions, while folder/document edges
remain visually distinguishable in every supported theme and accent.

## Scope and boundaries

In scope: workspace action semantic roles, localized action hints, Tooltip and
accessible-description projection, file/folder QSS edges, static contract,
diagnostics, package evidence, and delivery records.

Out of scope: file picker behavior, workspace containment, tree click/double
click/Enter routing, asynchronous document opening, Settings schema,
persistence, MainWindow/application ownership, native screen-reader testing,
and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Explicit folder/file opening outcome |
| Developer 1 | parent | WorkspacePanel action roles and accessibility projection |
| Developer 2 | parent | i18n/theme/static contract integration |
| QA | parent | Source probe, matrix, diagnostics, package, archive, and handoff |

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — assigns action roles and
  refreshes localized label, Tooltip, accessible name, and description.
- `src/quillforge/presentation/i18n.py` — adds four English/Chinese action
  hints.
- `src/quillforge/presentation/theme.py` — adds semantic folder/document edge
  selectors.
- `scripts/audit_presentation_contracts.py` — guards role, locale, assistive,
  and QSS contracts.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- Existing file/folder signals and asynchronous open admission are the sole
  behavior boundary; this slice only improves discoverability and semantics.
- The action helper is local to `WorkspacePanel`; no global notification or
  Locale service was introduced.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D308 workspace affordance probe | PASS — four roles, localized hints, Tooltip/accessibility helper, and QSS selectors |
| D308 workspace edge matrix | PASS — 3 themes × 4 accents; minimum edge contrast 3.57:1 |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `BEE2B6E4E5709FFFABE9F99A441853F8AA675531BB8A411EE8023C95D24CC647`; 38,599,052 bytes; source `tree-sha256:2140b64f8066bfe2317af05ecae6f96fa791f1812abe952cce591defc491ae4b` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 169; qwindows 1; Qt6 DLLs 7; QScintilla 1; PyQt runtime hook 1 |

## Unrun checks and reason

Native EXE/Qt startup, screen-reader output, pixel rendering, focus traversal,
DPI, clean-machine behavior, real DLL loading, signing, installer, updater,
registry, and release-owner evidence remain unrun because the active policy
prohibits native software launch and external release actions. Unit tests,
mocks, fixtures, and test harnesses were not created or run.

## Known risks and limits

The source contract and edge matrix prove the intended presentation wiring but
not native Tooltip timing, screen-reader output, or perceived visual weight.
The independent review returned no conclusion after three bounded waits. The
release dossier remains no-go with historical artifact consistency gates open.

## Acceptance and evidence IDs

- Acceptance: `S348`
- Evidence: `D308-AFFORDANCE-PROBE=PASS`, `D308-QSS-MATRIX=PASS`,
  `D308-COMPILEALL=PASS`, `D308-RUFF=PASS`, `D308-FORMAT=PASS`,
  `D308-AUDIT=PASS`, source diagnostics, package identity, PE/archive checks,
  review records, and expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native Workspace interaction/screen-reader review and
  artifact-bound startup evidence if full UI or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `BEE2B6E4E5709FFFABE9F99A441853F8AA675531BB8A411EE8023C95D24CC647` /
  `38599052` bytes
- Source revision:
  `tree-sha256:2140b64f8066bfe2317af05ecae6f96fa791f1812abe952cce591defc491ae4b`

## Disposition

Accepted with limits. D308 is source-, contract-, contrast-, diagnostic-,
package-, archive-, and handoff-verified. Independent review, native action
interaction, clean-machine behavior, and remaining release gates remain open.
