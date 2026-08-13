# Handoff: 2026-08-11-d48-document-tab-state

| Field | Value |
|---|---|
| ID | `2026-08-11-d48-document-tab-state` |
| Delivery / slice | `D48 / ARCH-38 / UI-34 Document-tab state clarity` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T07:00:00+08:00` |

## User outcome

Modified documents now have a visible authored document-plus-dot marker in the
tab rail while retaining the existing asterisk title cue. The marker follows
the selected theme/accent, refreshes after settings changes, and remains
presentation-only; document dirty/save/recovery/close policy is unchanged.

## Scope and boundaries

### In scope

- `IconKey.MODIFIED` authored vector glyph.
- `DocumentTabSurface` modified-marker projection, index alignment, and theme
  refresh.
- MainWindow integration at existing tab add/dirty/title/theme routes.
- Static architecture, review, handoff, package, and release evidence.

### Out of scope

- Document model or persistence schema, dirty-state calculation, save/recovery
  policy, tab close/current behavior, custom delegates, animation, runtime
  visual acceptance, screenshots, clean-machine/cross-machine evidence,
  signing, installer, updater, legal, support, and release-owner approval.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent architect | Integration, boundary decision, verification, and handoff |
| Project Manager | Project Manager (QuillForge) | Plan, dependencies, risks, and release status |
| Product | User / product owner | Clear unsaved-state hierarchy in the editor shell |
| Developer | Parent architect | Smallest source change in icons/tab surface/MainWindow projection |
| QA | Parent architect | Read-only source/static verification and unrun evidence |
| Independent review | Parfit the 2nd / Luna max | Read-only review; no conclusion returned |

## Decisions and constraints

- `DocumentTabSurface` owns only visual marker projection. MainWindow remains
  the owner of `state.dirty or editor.is_modified()` and all document policy.
- Tab and marker arrays are updated together; existing title, close, current,
  save, recovery, and lifecycle contracts remain intact.
- Theme changes call an explicit `refresh_icons()` route; new tabs use the
  current widget palette.
- Beauvoir the 2nd / Luna max was consulted as the required architecture role;
  two bounded windows returned no conclusion, so no architecture PASS is
  claimed. Parfit the 2nd / Luna max independent review also returned no
  conclusion after two bounded waits.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch is not authorized; source, compilation, packaging, and
  release-handoff evidence are the permitted validation boundary.

## Changed files and modules

- `src/quillforge/presentation/icons.py` — authored modified-document glyph.
- `src/quillforge/presentation/document_tab_surface.py` — marker projection,
  index alignment, and theme refresh.
- `src/quillforge/presentation/main_window.py` — existing dirty/title/theme
  routes project state to the presentation surface.
- `docs/adr/0073-document-tab-state-clarity.md` — architecture decision.
- `docs/agent-team/reviews/D48-document-tab-state-parent-review.md` and
  `D48-document-tab-state-independent-review.md` — review evidence.
- `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, and
  `docs/specs/enterprise-architecture-migration.md` — architecture projection.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `tasks/plan.md`, and `tasks/todo.md` — traceability.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D48 document-tab state source probe | `PASS` | Marker, index alignment, theme refresh, and policy ownership. |
| `uv run python -m compileall -q src` | `PASS` | Authorized static compilation only. |
| `uv run ruff check src` | `PASS` | No diagnostics. |
| `uv run ruff format --check src` | `PASS` | Source is formatted. |
| `pwsh -NoProfile -File scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff and required sections agree. |
| `scripts\check.ps1` | `PASS` | Repository static, JSON, lint, format, and handoff checks pass. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | 10 open gates and three known report-binding failures remain; human handoff identity matches. |

## Unrun checks and reason

- Native tab icon metrics, screen-reader output, runtime tab interaction,
  queued callback timing, screenshots, accessibility, DPI, clean-machine,
  cross-machine, permission/disk pressure, hard-power, signing, installer,
  updater, legal, support, and release-owner evidence — outside the current
  no-launch or external-authorization boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source evidence cannot prove native Qt tab icon sizing or rendering on
  every Windows DPI configuration.
- Independent review has no conclusion; the parent review records evidence and
  limits without upgrading that status.
- The portable candidate is unsigned and not an installer; release remains
  `NO-GO` until the external gates are closed.

## Acceptance and evidence IDs

- Acceptance: `D48-AC01`, `S77`.
- Evidence: ADR-0073, parent/independent reviews, D48 source probe, static
  checks, handoff verifier, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: obtain independent review or authorized runtime visual evidence before
  strengthening the tab marker claim; continue the next bounded coordinator
  slice only after preserving this boundary.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `BB0FF20D47D4AA2A5C168D7D9AE5D4AA239D6856AC04336257475CB5F0BA1984` /
  `38,417,875` bytes; root/dist identity matches.
- Source revision:
  `tree-sha256:42a826ef33be560d8d7f6afd0b702a70f3981452f9a109badefeeb0b72eb16e5`.
- Packaging note: unsigned portable one-file candidate; installer, updater,
  file associations, clean-machine evidence, and release approval remain open.

## Disposition

`accepted-with-limits`: the document-tab modified-state projection is integrated
and statically verified; independent review, native runtime, and release-owner
gates remain conditions for later work.
