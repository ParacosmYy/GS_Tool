# Handoff: 2026-08-12-ui-71-workspace-search-query-card

| Field | Value |
|---|---|
| ID | 2026-08-12-ui-71-workspace-search-query-card |
| Delivery / slice | UI-71 / ARCH-123 workspace-search query card |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T08:00:00+08:00 |

## User outcome

The workspace search dialog now presents its query controls in a readable
modern card with distinct surface, hover, and focus states. Search behavior,
keyboard activation, cancellation, diagnostics, localization, and results are
unchanged.

## Scope and boundaries

### In scope

- `QFrame#workspaceSearchQueryCard` around the existing query row.
- Scoped centralized QSS for the card, query field, and case checkbox.
- Static theme/accent projection, signal-preservation probe, package, and
  records.

### Out of scope

- No search service, query validation, TaskRunner, result list, diagnostics,
  status policy, locale catalog, font settings, motion, theme/accent schema,
  or application policy rewrite.
- No QApplication/style-engine, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Carver the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Carson the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_search_dialog.py` — query card
  presentation wrapper only.
- `src/quillforge/presentation/theme.py` — scoped token QSS only.
- `docs/adr/0185-workspace-search-query-card.md`
- `docs/agent-team/reviews/UI-71-workspace-search-query-card-parent-review.md`
- `docs/agent-team/reviews/UI-71-workspace-search-query-card-independent-review.md`

## Decisions and constraints

- The centralized theme stylesheet remains the single visual owner.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `UI71-WORKSPACE-SEARCH-QUERY-CARD-PROBE=PASS: 12 theme/accent projections`
- `UI71-SEARCH-SIGNAL-PRESERVATION-PROBE=PASS`
- `UI71-QSS-SCOPE-PROBE=PASS`
- `UI71-COMPILEALL=PASS`
- `UI71-RUFF=PASS`
- `UI71-FORMAT=PASS`
- `UI71-CHECK=PASS`
- `UI71-VERIFY-HANDOFF=PASS`
- `UI71-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect conclusion — child window timed out; recorded as `NO_CONCLUSION`.
- Independent review conclusion — child window timed out; recorded as
  `NO_CONCLUSION`, not PASS.
- QApplication/style-engine rendering, installed fonts, DPI, accessibility
  tooling, runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Native QSS layout/metrics can vary with style engine, installed fonts, and
  DPI; these remain runtime evidence items.
- Carver architecture and Carson independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S189`, `UI-71-AC01`.
- Evidence: ADR-0185, query-card/selector/signal probes, parent and independent
  review records, simplification assessment, static checks, package identity,
  handoff/index/register checks, expected release NO-GO, and explicit runtime
  limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `7FBD19F99A5143460A164A7E18037D74AB8F5A3972BE4029C6BB2AA8140FCF52`
- Size: `38533406` bytes
- Source revision: `tree-sha256:47d9f02397910a13c2df7f12f863ffd65957150d17bb76633e5440d96ca472a4`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace search query hierarchy is refined through the
existing token stylesheet; native rendering/runtime/release evidence remains
open.
