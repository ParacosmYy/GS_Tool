# Handoff: 2026-08-10-d74-settings-preview-surface

| Field | Value |
|---|---|
| ID | `2026-08-10-d74-settings-preview-surface` |
| Delivery / slice | `D74 / UI-47 settings preview surface boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

The settings appearance preview remains available, while its visual composition
now lives behind a focused presentation surface. The Settings dialog remains a
clear form coordinator: it projects current theme, accent, interface font,
interface size, and locale into the preview; Save/Cancel, persistence, global
theme timing, editor settings, and motion policy remain unchanged.

## Scope and boundaries

### In scope

- Extraction of the preview widget tree and `project(...)` projection boundary.
- Presentation dependency-direction and no-second-state-model probe.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No settings schema, application service, persistence, MainWindow, editor,
  locale policy, filesystem, plugin, process, or domain-model change.
- No native runtime, screenshot, accessibility-driver, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Descartes the 3rd / Luna max | Pre-edit boundary consultation; no conclusion after bounded waits |
| Independent review | Euler the 3rd / Luna max | Read-only source review; no conclusion after bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/settings_preview.py` — focused preview surface.
- `src/quillforge/presentation/settings_dialog.py` — surface composition and
  one-way current-control projection.
- D73's canonical `theme.py` and `i18n.py` seams remain unchanged in ownership.
- ADR, reviews, acceptance/register/index, roadmap, and task records.

## Decisions and constraints

- The new surface is presentation-only and has one `project(...)` method;
  SettingsDialog remains the source of pending settings values.
- `theme.py` remains the sole token/QSS owner and application/domain code has
  no dependency on the new surface.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D74 presentation boundary probe | `PASS` | Surface contract and dependency direction. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation sources. |
| Full compileall / Ruff / format | `PASS` | Final synchronization validation. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native Qt parent/layout/rendering, modal interaction, keyboard traversal,
  accessibility, installed-font fallback, DPI, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static dependency evidence does not prove native Qt parent/layout behavior,
  stylesheet precedence, or visual output on every Windows DPI configuration.
- Both delegated windows returned no conclusion; no child PASS is claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D74-AC01`, `S103`.
- Evidence: ADR-0099, presentation boundary probe, parent/independent reviews,
  static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible/architecture gap or obtain
  authorized runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `0EFEE62E6BB076F12E50245F06169AD6CD28B2E850B81BB713756B85FF69F1DF` / `38,448,268` bytes.
- Source revision: `tree-sha256:4b9d70043ad0803575b826a9b673f1eb70293e325de1b89cb1ac4c68c29e7a50`.

## Disposition

`accepted-with-limits`: the settings preview surface extraction is integrated
with a one-way presentation contract and package identity is recorded, while
runtime and release gates remain open.
