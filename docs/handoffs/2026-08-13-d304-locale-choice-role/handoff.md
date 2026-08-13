# Handoff: 2026-08-13-d304-locale-choice-role

| Field | Value |
|---|---|
| ID | `2026-08-13-d304-locale-choice-role` |
| Delivery / slice | `D304 / UI-123 / ARCH-274 Locale choice role` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T09:30:00+08:00` |

## User outcome

The Settings language selector now reads as a first-class locale choice with a
clear normal, hover, focus, open, and disabled hierarchy. The existing English
and Simplified Chinese choices, immediate label refresh, snapshot, persistence,
and keyboard behavior remain unchanged.

## Scope and boundaries

In scope: presentation-only `localeChoice` metadata, centralized locale-choice
QSS, cross-surface edge resolution, audit coverage, package evidence, and
delivery records.

Out of scope: locale values, item data, translations, schema, persistence,
signals, `set_locale()` behavior, keyboard routing, application ownership,
native rendering, and release-gate closure.

Existing object identity, `currentData()`, `currentIndexChanged`, locale
reprojection, labels, and `SettingsSnapshot` reads remain unchanged. The role
property is not persisted.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultations | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Locale-choice hierarchy and readability outcome |
| Developer 1 | parent | SettingsDialog semantic metadata |
| Developer 2 | parent | Theme token/QSS and canonical audit |
| QA | parent | Matrix, source diagnostics, package, archive, and handoff checks |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — declares the localeChoice
  presentation role before existing language item setup.
- `src/quillforge/presentation/theme.py` — owns localeChoice state QSS and its
  resolved edge token.
- `scripts/audit_presentation_contracts.py` — guards ordering, state coverage,
  edge token use, and legacy-selector removal.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- `presentation.theme_tokens` remains the pure color-resolution owner;
  `presentation.theme` remains the sole QSS owner.
- `localeChoice` is presentation-only metadata and is not persisted.
- Existing locale values, item data, signals, locale refresh, snapshot,
  persistence, keyboard routing, and application ownership remain in their
  current modules.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D304 source role contract | PASS — localeChoice, item data/current data, no legacy active IDs |
| D304 compatibility probe | PASS — currentIndexChanged, set_locale, locale labels, snapshot path unchanged |
| D304 QSS matrix | PASS — 3 themes × 4 accents; text minimum 5.14:1; resolved edge minimum 3.62:1 |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `99891A57F00DC995EC7D8A4EE1110962E71470FC911AEB09A1B88BB8B1E9A2D5`; 38,595,652 bytes; source `tree-sha256:ecbb487999c1298da8e81e5ebecc3c9f54008bf2e1f61aad31a22dee7c5201e6` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 166; qwindows 1; Qt6 DLLs 29; QScintilla 1; PyQt runtime hook 1; application modules 4 |

## Unrun checks and reason

Native EXE/Qt startup, pixel rendering, focus/accessibility, DPI, font
fallback, clean-machine behavior, real DLL loading, signing, installer,
updater, registry, and release-owner evidence remain unrun because the active
policy prohibits native software launch and external release actions. Static
token/stylesheet evidence does not prove native painting or perceived visual
weight. Unit tests, mocks, fixtures, and test harnesses were not created or
run.

## Known risks and limits

The resolved 3:1 edge floor is a token calculation rather than a native pixel
sample; DPI, platform styles, and font rasterization can change perceived
weight. The independent review returned no conclusion, and release gates
remain open.

## Acceptance and evidence IDs

- Acceptance: `S344`
- Evidence: `D304-LOCALE-CHOICE-ROLE=PASS`,
  `D304-LOCALE-CHOICE-CONTRACT=PASS`, `D304-QSS-MATRIX=PASS`, source
  diagnostics, package identity, PE/archive checks, review records, and
  expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native visual/focus/accessibility review and artifact-bound
  startup evidence if pixel-level or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `99891A57F00DC995EC7D8A4EE1110962E71470FC911AEB09A1B88BB8B1E9A2D5` /
  `38595652` bytes
- Source revision:
  `tree-sha256:ecbb487999c1298da8e81e5ebecc3c9f54008bf2e1f61aad31a22dee7c5201e6`

## Disposition

Accepted with limits. D304 is source-, semantic-contract-, contrast-matrix-,
diagnostic-, package-, archive-, and handoff-verified. Independent review,
native visual rendering, clean-machine behavior, and remaining release gates
remain open.
