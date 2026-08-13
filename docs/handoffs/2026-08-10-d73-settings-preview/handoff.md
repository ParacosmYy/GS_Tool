# Handoff: 2026-08-10-d73-settings-preview

| Field | Value |
|---|---|
| ID | `2026-08-10-d73-settings-preview` |
| Delivery / slice | `D73 / UI-46 settings appearance preview` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

Settings now includes a live, readable appearance preview. Theme, accent,
interface font, and interface size choices are visible before Save through
canvas, panel, selected, accent, and sample-text states. Locale changes
retranslate the preview without losing selections. Save, Cancel, persistence,
global application timing, editor settings, and motion policy are unchanged.

## Scope and boundaries

### In scope

- Presentation-only settings preview card and bilingual labels.
- Canonical theme token/style projection for pending choices.
- All-theme/all-accent contrast and selector evidence.
- Source, static, package, handoff, and release evidence.

### Out of scope

- No immediate global theme application, persistence, settings schema, editor
  behavior, MainWindow policy, application service, filesystem, plugin,
  process, or domain-model change.
- No native runtime, screenshot, accessibility-driver, clean-machine, signing,
  installer, updater, legal, support, or release-owner change.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Hume the 3rd / Luna max | Pre-edit boundary consultation; no conclusion after bounded waits |
| Independent review | Socrates the 3rd / Luna max | Read-only source review; no conclusion after bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — public token resolver and scoped
  preview stylesheet; no duplicate palette.
- `src/quillforge/presentation/settings_dialog.py` — preview object tree,
  locale projection, and control-change refresh only.
- `src/quillforge/presentation/i18n.py` — bilingual preview labels/metadata.
- ADR, reviews, acceptance/register/index, roadmap, and task records.

## Decisions and constraints

- `theme.py` remains the sole token/QSS owner; the dialog remains the sole
  pending-preview projection owner.
- Preview styling uses bounded settings catalog values; it does not render
  user-provided paths, HTML, or external resources.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 application/presentation code. Embedded C/C++
  assurance and vendor manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D73 preview contrast/selector/font probe | `PASS` | 3 themes × 4 accents and pending font/size projection. |
| Targeted compileall / Ruff / format | `PASS` | Changed presentation sources. |
| Full compileall / Ruff / format | `PASS` | Final synchronization validation. |
| `scripts\verify_handoff.ps1` | `PASS` | Indexed handoff, status, required sections, and policy contract. |
| `scripts\check.ps1` | `PASS` | Repository checks, notice inventory, handoff, and formatting. |
| `scripts\package.ps1` | `PASS` | Root/dist candidate identity recorded below. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing report bindings and external gates remain open. |

## Unrun checks and reason

- Native QSS rendering, dialog geometry, keyboard traversal, accessibility,
  installed-font fallback, DPI, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and
  release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static token/contrast evidence does not prove native Qt stylesheet precedence,
  layout metrics, or actual installed-font selection on every Windows DPI
  configuration.
- Both delegated windows returned no conclusion; no child PASS is claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D73-AC01`, `S102`.
- Evidence: ADR-0098, preview contrast/selector/font probe, parent/independent
  reviews, static checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next distinct user-visible gap or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `FC7159659A0A195027174BC2B18A2C8C6138EC505BAB3B6F5E3D9735D9BA3A4A` / `38,446,079` bytes.
- Source revision: `tree-sha256:07cbeb357e7a92694898587eac7aa8a853f2b36fc709fb3bb731ddab8b42f2a9`.

## Disposition

`accepted-with-limits`: the settings appearance preview is integrated through
the canonical presentation token boundary and package identity is recorded,
while runtime and release gates remain open.
