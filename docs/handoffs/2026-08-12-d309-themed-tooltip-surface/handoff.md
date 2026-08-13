# Handoff: 2026-08-12-d309-themed-tooltip-surface

| Field | Value |
|---|---|
| ID | `2026-08-12-d309-themed-tooltip-surface` |
| Delivery / slice | `D309 / UI-128 / ARCH-279 Theme-aware Tooltip surface` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:50:00+08:00` |

## User outcome

All QuillForge tooltips now use the selected theme's surface, typography,
border, readable foreground, and accent edge. Workspace file/folder hints and
future Tooltip-bearing controls share one modern, high-contrast treatment.

## Scope and boundaries

In scope: centralized QToolTip QSS, token-bound edge fallback, contrast/static
contract, package evidence, and delivery records.

Out of scope: tooltip text/content, locale catalog, widget behavior, business
logic, Settings persistence, animation policy, native Tooltip interaction,
screen-reader certification, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Modern, readable Tooltip outcome |
| Developer 1 | parent | Central `_stylesheet()` Tooltip rule |
| Developer 2 | parent | Static contract and contrast evidence |
| QA | parent | Matrix, compile/Ruff/format, package, archive, and handoff |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — adds token-bound global QToolTip
  styling and readable edge fallback.
- `scripts/audit_presentation_contracts.py` — adds Tooltip selector and matrix
  checks.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- Tooltip styling remains in the existing centralized theme owner; no widget
  or service acquires a stylesheet responsibility.
- Text and edge contrast are checked independently; color is supplementary to
  the tooltip's text content.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive evidence are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D309 Tooltip source probe | PASS — selector/token/fallback/centralization/behavior boundary |
| D309 Tooltip contrast matrix | PASS — 3 themes × 4 accents; text 11.16:1 minimum, edge 3.65:1 minimum |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| package identity | PASS — root/dist SHA `909A47FF0D952DB7FC8F91279982BDB4B67E744756BD2AE7B91B092EE944BFC5`; 38,598,697 bytes; source `tree-sha256:f6c6ca424c05adbd10d1c58ea3e00b34e94f07afb559ae854956879e7a13ba04` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 169; qwindows 1; Qt6 DLLs 7; QScintilla 1; PyQt runtime hook 1 |

## Unrun checks and reason

Native EXE/Qt startup, actual Tooltip painting, screen-reader output, focus
traversal, DPI, clean-machine behavior, real DLL loading, signing, installer,
updater, registry, and release-owner evidence remain unrun because the active
policy prohibits native software launch and external release actions. Unit
tests, mocks, fixtures, and test harnesses were not created or run.

## Known risks and limits

The source contract and token matrix prove the intended QSS and contrast
resolution but not native Tooltip placement, timing, font fallback, or visual
weight. The independent review returned no conclusion after three bounded
waits. The release dossier remains no-go with historical artifact consistency
gates open.

## Acceptance and evidence IDs

- Acceptance: `S349`
- Evidence: `D309-SOURCE-PROBE=PASS`, `D309-QSS-MATRIX=PASS`,
  `D309-COMPILEALL=PASS`, `D309-RUFF=PASS`, `D309-FORMAT=PASS`,
  `D309-AUDIT=PASS`, package identity, PE/archive checks, review records, and
  expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native Tooltip rendering/accessibility review and
  artifact-bound startup evidence if full UI or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `909A47FF0D952DB7FC8F91279982BDB4B67E744756BD2AE7B91B092EE944BFC5` /
  `38598697` bytes
- Source revision:
  `tree-sha256:f6c6ca424c05adbd10d1c58ea3e00b34e94f07afb559ae854956879e7a13ba04`

## Disposition

Accepted with limits. D309 is source-, contract-, contrast-, package-,
archive-, and handoff-verified. Independent review, native Tooltip behavior,
clean-machine behavior, and remaining release gates remain open.
