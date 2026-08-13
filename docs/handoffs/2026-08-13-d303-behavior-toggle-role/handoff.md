# Handoff: 2026-08-13-d303-behavior-toggle-role

| Field | Value |
|---|---|
| ID | `2026-08-13-d303-behavior-toggle-role` |
| Delivery / slice | `D303 / UI-122 / ARCH-273 Behavior toggle role` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T08:30:00+08:00` |

## User outcome

Settings wrapping, line-number, and motion checkboxes now read as one coherent
behavior-control group. Editor controls retain a pink tone edge, the motion
control retains an interface/cyan edge, and normal/hover/focus/checked/disabled
states have an explicit hierarchy. The existing settings behavior is retained.

## Scope and boundaries

In scope: presentation-only `behaviorToggle` role/tone metadata, centralized
role-based QSS, safe cross-surface edge resolution, audit coverage, package and
handoff evidence.

Out of scope: settings schema/values, persistence, locale, editor projection,
motion policy, signals, keyboard routing, labels, generic indicator semantics,
application ownership, native rendering, and release-gate closure.

Existing object names, checked values, labels, signals, `SettingsSnapshot`
reads, persistence paths, and keyboard behavior remain unchanged. Dynamic
properties are not persisted.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultations | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Behavior-control hierarchy and readability outcome |
| Developer 1 | parent | SettingsDialog semantic metadata |
| Developer 2 | parent | Theme token/QSS and canonical audit |
| QA | parent | Matrix, source diagnostics, package, archive, and handoff checks |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — declares the shared role
  and editor/interface tones before existing value setup.
- `src/quillforge/presentation/theme_tokens.py` — resolves an edge color across
  all state surfaces with the existing 3:1 policy.
- `src/quillforge/presentation/theme.py` — owns shared behavior-toggle QSS and
  tone-specific state edges.
- `scripts/audit_presentation_contracts.py` — guards ordering, state coverage,
  disabled tone cues, and legacy-selector removal.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- `presentation.theme_tokens` remains the pure color-resolution owner;
  `presentation.theme` remains the sole QSS owner.
- `settingsRole` and `settingsTone` are presentation-only metadata and are not
  persisted or exposed through the settings schema.
- Existing checkbox identity, value reads, signals, persistence, keyboard
  routing, and application ownership remain in their current modules.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Review and architecture

Architecture consultation: `NO_CONCLUSION` after three bounded waits.
Independent review: `NO_CONCLUSION` after three bounded waits; no independent
PASS is claimed. Parent review: `PASS`. Simplification assessment: `PASS`.

Qt Style Sheets and WCAG 2.2 SC 1.4.11 are public engineering references for
the Python 3.12/PyQt6 desktop UI. Embedded public-vendor applicability is N/A:
no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code changed. No
manufacturer, MISRA, ISO 26262, ASPICE, certification, or private corporate
standard claim is made.

## Verification commands and results

| Evidence | Result |
|---|---|
| D303 source role contract | PASS — 3 controls, shared role, editor/interface tones, no legacy active IDs |
| D303 compatibility probe | PASS — checked values, snapshot reads, indicators, signals, persistence, keyboard ownership unchanged |
| D303 QSS matrix | PASS — 3 themes × 4 accents; text minimum 5.14:1; resolved edge minimum 3.02:1 |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `14C4791E265CDF30A4C012F6F52301192AEDF9A47D8D9B3E7A8B286C8984289D`; 38,595,633 bytes; source `tree-sha256:4447fbca96be0e997b1ef7a1acf991cc2ed28a7ccc099dc56025fd4cfadce9d8` |
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
weight. The independent review returned no conclusion, and release gates remain
open.

## Acceptance and evidence IDs

- Acceptance: `S343`
- Evidence: `D303-BEHAVIOR-TOGGLE-ROLE=PASS`,
  `D303-BEHAVIOR-TOGGLE-CONTRACT=PASS`, `D303-QSS-MATRIX=PASS`, source
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
  `14C4791E265CDF30A4C012F6F52301192AEDF9A47D8D9B3E7A8B286C8984289D` /
  `38595633` bytes
- Source revision:
  `tree-sha256:4447fbca96be0e997b1ef7a1acf991cc2ed28a7ccc099dc56025fd4cfadce9d8`

## Disposition

Accepted with limits. D303 is source-, semantic-contract-, contrast-matrix-,
diagnostic-, package-, archive-, and handoff-verified. Independent review,
native visual rendering, clean-machine behavior, and remaining release gates
remain open.
