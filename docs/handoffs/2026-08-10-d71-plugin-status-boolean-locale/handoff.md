# Handoff: 2026-08-10-d71-plugin-status-boolean-locale

| Field | Value |
|---|---|
| ID | `2026-08-10-d71-plugin-status-boolean-locale` |
| Delivery / slice | `D71 / UI-44 plugin status boolean locale projection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Plugin Status tooltips now display enabled/active truth values in the selected
locale: `true/false` for English and `是/否` for Chinese. Plugin IDs,
versions, permissions, errors, lifecycle labels, selection, and enable/disable
behavior remain unchanged.

## Scope and boundaries

### In scope

- Two centralized boolean presentation strings.
- Tooltip-only locale projection.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No `PluginRuntimeStatus` contract, plugin lifecycle, trust, enablement,
  filesystem, process, persistence, signal, or MainWindow policy change.
- No application-layer locale dependency or new status view-model.
- No native runtime, screenshot, accessibility-driver, clean-machine,
  signing, installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Mill the 3rd / Luna max | Cross-module boundary consultation; no conclusion after two bounded waits |
| Independent review | Ampere the 3rd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — bilingual boolean values.
- `src/quillforge/presentation/plugin_status_dialog.py` — tooltip projection.
- `docs/adr/0096-plugin-status-boolean-locale.md` and D71 review records —
  decision, review, and simplification evidence.

## Decisions and constraints

- i18n owns the vocabulary; PluginStatusDialog owns Qt tooltip projection;
  application status remains typed and locale-free.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D71 plugin-status locale probe | `PASS` | Both boolean states, both locales, and dynamic values. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation source. |
| Full compileall / Ruff / format | `PASS` | Source and documentation synchronization completed. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native tooltip rendering, live locale switching, keyboard traversal,
  accessibility, DPI, fonts, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static tooltip evidence does not prove native rendering on every Qt style or
  DPI combination.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D71-AC01`, `S100`.
- Evidence: ADR-0096, plugin-status locale probe, parent/independent reviews,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible localization/UI gap or
  obtain authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `B17593EA91535FCE7BFD08071EE430EC1A77760D296373DF14C59110A6169F97` / `38,443,950` bytes.
- Source revision: `tree-sha256:c4dbfd62d9dcfc77b44919f4ccc506f8974be608788d0d7a45e69930195a8ed1`.

## Disposition

`accepted-with-limits`: Plugin Status boolean locale projection is integrated
and the package identity is recorded, while runtime and release gates remain
open.
