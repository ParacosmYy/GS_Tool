# Handoff: 2026-08-10-d60-ui-36-tab-selection

| Field | Value |
|---|---|
| ID | `2026-08-10-d60-ui-36-tab-selection` |
| Delivery / slice | `D60 / UI-36 document-tab selection hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T19:00:00+08:00` |

## User outcome

The shell has stronger visual hierarchy where it matters most: active tabs,
selected-tab hover, the tab rail boundary, and workspace dock titles now have
clear, theme-consistent emphasis without changing behavior.

## Scope and boundaries

### In scope

- Central QSS selected/hover/focus/rail/dock visual hierarchy.
- Existing token reuse and 12-pair static contrast evidence.
- Static, package, handoff, and release evidence.

### Out of scope

- No tab behavior, signals, metadata, settings, locale, motion, editor,
  warning/action semantics, runtime event, or application policy change.
- No new theme token, style engine, test asset, runtime launch, screenshot,
  clean-machine, signing, installer, updater, legal, support, or release-owner
  evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Darwin the 2nd / Luna max | UI boundary consultation; no conclusion after two bounded waits |
| Independent review | Hubble the 2nd / Luna max | Read-only source review; no conclusion after two bounded waits |
| Parent | Architect | Sole writer, integration, final review, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — strengthens central tab/dock QSS.
- `docs/adr/0085-document-tab-selection-hierarchy.md` — decision.
- `docs/agent-team/reviews/D60-ui-36-tab-selection-parent-review.md` and
  `D60-ui-36-tab-selection-independent-review.md` — review records.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/handoffs/index.json`, `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`,
  `docs/specs/enterprise-architecture-migration.md`, `tasks/plan.md`, and
  `tasks/todo.md` — traceability.

## Decisions and constraints

- Central `ThemeColors` remains the only token owner; selected/hover states
  reuse existing contrast-safe endpoints and do not use the warning gold fill.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and
  release-handoff checks are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A`.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| UI-36 tab selection hierarchy probe | `PASS` | Selector/token boundaries remain centralized. |
| UI-36 tab selection contrast probe | `PASS` | 3 themes × 4 accents, all selected pairs ≥ 4.5:1. |
| Targeted compileall / Ruff / format | `PASS` | Changed theme source. |
| Full compileall / Ruff / format | `PASS` | Run after final documentation/package sync. |
| `scripts\verify_handoff.ps1` | `PASS` | Handoff indexed and synchronized. |
| `scripts\check.ps1` | `PASS` | Acceptance/register synchronized. |
| `scripts\package.ps1` | `PASS` | Root/dist portable candidates match. |
| `scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO` | Existing external gates and stale report bindings remain. |

## Unrun checks and reason

- Native QSS rendering/specificity, focus traversal, screenshots, accessibility,
  DPI, fonts, runtime startup, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Hardware/firmware evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static contrast does not prove native Qt style rendering or font metrics.
- Both delegated review windows returned no conclusion; no child PASS is
  claimed.
- The portable candidate remains unsigned and release remains `NO-GO`.

## Acceptance and evidence IDs

- Acceptance: `D60-AC01`, `S89`.
- Evidence: ADR-0085, source/QSS probes, parent/independent reviews, static
  checks, package identity, and expected release NO-GO.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded UI/architecture slice or obtain authorized
  runtime/release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `F9F6FAD70A3A2A207149424EAA3E595E574202AAC5BDA0E42FC8FB3E825CA4BF` / `38,434,509` bytes; root/dist identity matches.
- Source revision: `tree-sha256:b2d95294848f3cca30f53805e08952da2f2664c19d6d8fd3d8e262a0792e182e`.

## Disposition

`accepted-with-limits`: the centralized visual hierarchy is integrated and
static-verified; package identity and full traceability are synchronized after
the final package, while runtime visual and release gates remain open.
