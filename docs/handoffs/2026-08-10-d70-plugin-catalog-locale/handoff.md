# Handoff: 2026-08-10-d70-plugin-catalog-locale

| Field | Value |
|---|---|
| ID | `2026-08-10-d70-plugin-catalog-locale` |
| Delivery / slice | `D70 / UI-43 plugin catalog locale projection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Plugin Catalog rows and tooltips now follow the selected English/简体中文
locale instead of leaving stable status and field labels in English. Plugin
names, IDs, versions, paths, hashes, permissions, entrypoint metadata, and
free-form diagnostics remain exact and searchable. An empty catalog also
refreshes its localized state.

## Scope and boundaries

### In scope

- Centralized plugin catalog enum/field/reason vocabulary.
- Locale-aware row, tooltip, and empty-state projection.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No catalog rescan, entry mutation, plugin trust/approval/execution,
  filesystem, process, persistence, signal, or MainWindow policy change.
- No application-layer locale dependency or new catalog view-model.
- No native runtime, screenshot, accessibility-driver, clean-machine,
  signing, installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Schrodinger the 3rd / Luna max | Cross-module boundary consultation; no conclusion after two bounded waits |
| Independent review | McClintock the 3rd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — catalog vocabulary and raw-value
  fallback helper.
- `src/quillforge/presentation/plugin_catalog_dialog.py` — locale-aware row,
  tooltip, and empty-state projection.
- `docs/adr/0095-plugin-catalog-locale-projection.md` and D70 review records —
  decision, review, and simplification evidence.

## Decisions and constraints

- i18n owns stable vocabulary; the dialog owns Qt projection; application
  catalog contracts remain locale-free and immutable.
- Dynamic paths, names, IDs, versions, permissions, API values, entrypoints,
  hashes, and raw errors remain unmodified.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D70 catalog locale probe | `PASS` | zh-CN/en-US rows, tooltips, stable labels, and dynamic values. |
| D70 catalog value probe | `PASS` | Known enum localization, unknown raw fallback, and en-US identity. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation source. |
| Full compileall / Ruff / format | `PASS` | Source and documentation synchronization completed. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native QListWidget rendering, live locale switching, keyboard traversal,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static projection evidence does not prove native list metrics or every Qt
  style/DPI combination.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D70-AC01`, `S99`.
- Evidence: ADR-0095, catalog locale probe, parent/independent reviews,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible localization/UI gap or
  obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `0C36D873224461FA9F0F6B4AB5744E1CD0319258EC1BB1FA31CF9D778E64599C` / `38,443,687` bytes.
- Source revision: `tree-sha256:1acd43be1a5ada6c516ed72fb0d2efaee69bc62290f225553dc0a788187120b4`.

## Disposition

`accepted-with-limits`: Plugin Catalog locale projection is integrated and the
package identity is recorded, while runtime and release gates remain open.
