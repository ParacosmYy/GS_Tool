# Handoff: 2026-08-13-d301-semantic-settings-choice-role

| Field | Value |
|---|---|
| ID | `2026-08-13-d301-semantic-settings-choice-role` |
| Delivery / slice | `D301 / UI-120 / ARCH-271 Semantic Settings choice role` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T06:30:00+08:00` |

## User outcome

The Settings theme and accent controls keep D300's visible hierarchy while the
style boundary is now semantic and extensible. Both identity choices opt into
one `settingsRole="identityChoice"` presentation role, so future identity
choices do not need another duplicated normal/hover/focus/open selector set.

## Scope and boundaries

### In scope

- Dynamic-property binding for the existing theme and accent combo boxes.
- Centralized semantic QSS state ownership and its source audit contract.
- Preservation probe for values, item data, icons, signals, snapshot,
  persistence, keyboard behavior, and disabled styling.
- Contrast matrix, source diagnostics, package/archive evidence, reviews, and
  delivery records.

### Out of scope

- Settings schema/value definitions, persistence policy, locale resolution,
  theme resolution, preview behavior, keyboard routing, custom widgets,
  native rendering, screenshots, clean-machine behavior, signing, installer,
  updater, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultations | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | User-visible hierarchy and extensibility outcome |
| Developer 1 | parent | SettingsDialog semantic property binding |
| Developer 2 | parent | Canonical audit contract and package integration |
| QA | parent | Source, matrix, diagnostic, archive, identity, and handoff verification |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — assigns the shared
  `identityChoice` role before item population.
- `src/quillforge/presentation/theme.py` — projects the existing visual states
  through one semantic QSS selector while retaining disabled compatibility.
- `scripts/audit_presentation_contracts.py` — canonical state-table, property
  order, and legacy-selector contract.
- ADR, review, acceptance, delivery-register, roadmap, architecture, todo,
  plan, release handoff, and this handoff record.

## Decisions and constraints

- `presentation.theme` remains the sole QSS owner; no widget stylesheet or
  custom combo subclass is introduced.
- `settingsTheme`/`settingsAccent`, item order/data, icons, signals,
  `settings_snapshot()`, locale refresh, persistence, and application wiring
  remain compatible.
- The existing ID-based disabled rule is deliberately retained; only the
  D300 normal/hover/focus/open identity block becomes semantic.
- Shared checkout writer: parent only; no worktree or parallel writer was used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Review and source applicability

Initial architecture consultation: `NO_CONCLUSION` after three bounded waits.
Follow-up architecture review: `REVISE`; its stale-audit and duplicate-assertion
findings were applied, after which the canonical audit passed. Parent review:
`PASS`. Simplification assessment: `PASS`. Independent review:
`NO_CONCLUSION` after three bounded waits.

Qt 6.11.1 QObject and Qt Style Sheets documentation is an engineering
reference for Python 3.12/PyQt6 desktop presentation code. Public
embedded-vendor source applicability is N/A. No manufacturer, MCU, SDK, RTOS,
MISRA, ISO 26262, ASPICE, certification, or private ByteDance-standard claim
is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src scripts` | PASS | Source compiles. |
| `uv run ruff check src scripts` | PASS | Ruff clean. |
| `uv run ruff format --check src scripts` | PASS | 149 files formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Canonical semantic-role contract passes. |
| D301 semantic source probe | PASS | Role declaration precedes item population; shared QSS selector present; legacy active-state ID selectors absent. |
| D301 settings contract probe | PASS | Item/data/icon/signal/snapshot/persistence/keyboard source contracts unchanged. |
| D301 QSS matrix | PASS | 3 themes × 4 accents; normal/hover/focus/on; minimum checked contrast 8.64:1. |
| source `--diagnose-startup --report ...` | PASS | Composition/restore passed; no window/exec entered. |
| source `--diagnose-file-open README.md --report ...` | PASS | File-open path passed; no window/exec entered. |
| `.\scripts\package.ps1` | PASS | Portable candidate rebuilt. |
| package identity / PE/archive | PASS | Root/dist copies match; AMD64 PE32+ Windows GUI and Qt/Qsci payload inspected. |
| `.\scripts\check.ps1` | PASS | Project checks pass after handoff synchronization. |
| `.\scripts\verify_handoff.ps1` | PASS | Handoff/index/register/acceptance records validate. |
| `.\scripts\verify_release_handoff.ps1` | EXPECTED NO-GO | Three artifact-bound mechanical failures and ten open enterprise gates remain. |

## Unrun checks and reason

- Independent review conclusion — three bounded waits returned no result; no
  independent PASS is claimed.
- Native Qt/EXE rendering, screenshot, focus/accessibility, DPI, alternate
  style engines, clean-machine behavior, and real DLL-loader behavior —
  prohibited by `software_start_allowed=false`.
- Unit tests, mocks, fixtures, and test harnesses — excluded by project policy.
- Signing, installer, updater, registry, cross-machine, and release-owner
  evidence — outside this local UI slice.

## Known risks and limits

- Dynamic-property selector polish and `:on` painting are statically reasoned
  from the Qt contract, not visually exercised.
- The retained ID-based disabled rule is intentionally a compatibility seam;
  future identity roles must document whether disabled styling remains shared.
- Release remains no-go until authorized artifact-bound runtime and enterprise
  gates are refreshed for this candidate.

## Acceptance and evidence IDs

- Acceptance: `S341`
- Evidence: `D301-SEMANTIC-ROLE=PASS`, `D301-SEMANTIC-ROLE-PROBE=PASS`,
  `D301-SETTINGS-CONTRACT=PASS`, `D301-QSS-MATRIX=PASS`, source diagnostics,
  package identity, PE/archive checks, review records, and expected release
  no-go.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native visual/focus/accessibility review if pixel-level
  acceptance of the semantic identity choice role is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `BE724B944E3D207DF725A33FBD5ABA7C171B7C9B13EFB3C064968847BAE0C17E` / `38595342` bytes
- Source revision: `tree-sha256:d916510f42d100b5cfd4128c5638e866cf49af1df827ba5153f9dc69877e4a12`

## Disposition

Accepted with limits. D301 is source, semantic-contract, matrix, diagnostic,
package, PE/archive, and handoff verified; independent review, native visual
rendering, clean-machine behavior, and remaining release gates remain open.
