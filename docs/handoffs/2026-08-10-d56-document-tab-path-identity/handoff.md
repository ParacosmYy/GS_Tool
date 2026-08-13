# Handoff: 2026-08-10-d56-document-tab-path-identity

| Field | Value |
|---|---|
| ID | `2026-08-10-d56-document-tab-path-identity` |
| Delivery / slice | `D56 / ARCH-45 document-tab path identity boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T15:00:00+08:00` |

## User outcome

Document tab identity lookup is now owned by the tab projection boundary. The
enterprise architecture becomes easier to extend without moving document,
file, or startup policy out of MainWindow.

## Scope and boundaries

### In scope

- Canonical path lookup in `DocumentTabSurface`.
- MainWindow delegation for full-registry lookup.
- Explicit preservation of startup restore subset selection.
- Static, package, handoff, and release evidence.

### Out of scope

- No path-index cache, file opening, save behavior, session policy, domain
  contract change, UI styling, localization, or runtime event change.
- No Qt/EXE startup, screenshot, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Copernicus the 2nd / Luna max | Architecture review; no conclusion after two bounded waits |
| Independent review | Raman the 2nd / Luna max | Source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/document_tab_surface.py` — adds typed path
  identity lookup using the canonical domain helper.
- `src/quillforge/presentation/main_window.py` — delegates full-registry path
  lookup while retaining session-restore subset policy.
- `docs/adr/0081-document-tab-path-identity-boundary.md` — decision.
- `docs/agent-team/reviews/D56-document-tab-path-identity-parent-review.md`
  and `D56-document-tab-path-identity-independent-review.md` — review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability.

## Decisions and constraints

- The surface owns only projected tab identity/index lookup. MainWindow owns
  document services, startup barriers, restore subset selection, and user
  policy.
- `path_key()` remains the one canonical path identity helper; no duplicate
  normalization or cache was added.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D56 path identity boundary probe | `PASS` | Surface owns canonical lookup; MainWindow delegates. |
| D56 restore subset policy probe | `PASS` | Restore active-target subset logic remains in MainWindow. |
| Targeted compileall | `PASS` | Two changed source files. |
| Targeted Ruff | `PASS` | No diagnostics. |
| Targeted format check | `PASS` | Source formatted. |
| Full compileall / Ruff / format | `PASS` | Run after final documentation/package sync. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff indexed and synchronized. |
| `scripts\check.ps1` | `PASS` | Acceptance/register synchronized. |
| `scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing external gates and stale report bindings remain. |

## Unrun checks and reason

- Native Qt tab events, file-system case behavior across Windows volumes,
  screenshots, runtime startup, accessibility, DPI, fonts, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static delegation evidence cannot prove native Qt index/callback ordering.
- Independent review returned no conclusion; no child PASS is claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D56-AC01`, `S85`.
- Evidence: ADR-0081, source probes, parent/independent reviews, static
  checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow contract extraction or obtain
  authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `5DD9201399C8FC583DDDC0D970A8141A9B601E9F6AC75B26210C8D7B21CC0FB4` / `38,433,230` bytes; root/dist identity matches.
- Source revision: `tree-sha256:cdd72c1aa9799946e25d8983a69fb539dcbcd293b6a983c504980e1fc8e25a1c`.

## Disposition

`accepted-with-limits`: the source boundary is integrated and targeted-static
verified; package identity and full traceability remain pending until closure.
