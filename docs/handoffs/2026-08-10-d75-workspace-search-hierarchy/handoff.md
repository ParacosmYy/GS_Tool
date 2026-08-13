# Handoff: 2026-08-10-d75-workspace-search-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-10-d75-workspace-search-hierarchy` |
| Delivery / slice | `D75 / UI-48 workspace-search visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Find in Files now has a stronger scan order: the selected workspace root and
query are emphasized, result rows have clearer hover/focus/selection states,
and diagnostics are visually separated as warnings with an explicit toggle
state. Search behavior, data, cancellation, locale, and file activation are
unchanged.

## Scope and boundaries

### In scope

- Centralized workspace-search dialog/list QSS hierarchy.
- One semantic object name for the existing diagnostic toggle.
- All-theme/all-accent selector and contrast evidence.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No search service, query/result contract, TaskRunner, operation tracker,
  filesystem, diagnostic bound, cancellation, locale, plugin, process,
  application, or domain-model change.
- No native runtime, screenshot, accessibility-driver, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Arendt the 3rd / Luna max | Pre-edit boundary consultation; no conclusion after bounded waits |
| Independent review | Curie the 3rd / Luna max | Read-only source review; no conclusion after bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — scoped workspace-search QSS.
- `src/quillforge/presentation/workspace_search_dialog.py` — diagnostic toggle
  semantic object name only.
- ADR, reviews, acceptance/register/index, roadmap, and task records.

## Decisions and constraints

- `theme.py` remains the sole QSS/token owner; the dialog remains the owner of
  result/diagnostic object composition and existing signals.
- Warning foreground is derived from the existing warning background; accents
  remain edges/borders except for their checked/hover action state.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D75 search hierarchy/contrast probe | `PASS` | 3 themes × 4 accents, selectors, warning/selection/gold contrast. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation sources. |
| Full compileall / Ruff / format | `PASS` | Final synchronization validation. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native QSS rendering, dialog geometry, keyboard traversal, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS/contrast evidence does not prove native Qt specificity or visual
  output on every Windows DPI/font configuration.
- Both delegated windows returned no conclusion; no child PASS is claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D75-AC01`, `S104`.
- Evidence: ADR-0100, search hierarchy/contrast probe, parent/independent
  reviews, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible/architecture gap or obtain
  authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `DFA2C3CDAAB856D95213B6CA0E196A0962E8F550B420C505AFDEFB2163CF63F1` / `38,448,869` bytes.
- Source revision: `tree-sha256:a262139a478ac042b6b06083b3a7d22c94fa34de86ecb0fbd5f189f8960e56ea`.

## Disposition

`accepted-with-limits`: workspace-search hierarchy is integrated through scoped
theme tokens and a semantic toggle identity, while runtime and release gates
remain open.
