# Handoff: 2026-08-12-d307-settings-draft-status

| Field | Value |
|---|---|
| ID | `2026-08-12-d307-settings-draft-status` |
| Delivery / slice | `D307 / UI-126 / ARCH-277 Settings unsaved-draft status` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T23:00:00+08:00` |

## User outcome

Settings now shows whether the current draft matches the values used to open
the dialog. Changing theme, accent, language, fonts, sizes, styles, wrapping,
line numbers, motion, or RestoreDefaults produces an explicit localized
unsaved state; returning to the baseline shows a clean state.

## Scope and boundaries

In scope: a dialog-local baseline, localized clean/changed status label,
semantic QSS state, accessible status text, signal projection, static contract,
diagnostics, package evidence, and delivery records.

Out of scope: SettingsSnapshot/schema/persistence changes, SettingsService,
MainWindow/application projection, Save/Cancel behavior, new animation policy,
native screen-reader certification, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Visible unsaved-settings feedback outcome |
| Developer 1 | parent | SettingsDialog baseline/status projection |
| Developer 2 | parent | i18n/theme/static contract integration |
| QA | parent | Source probe, matrix, diagnostics, package, archive, and handoff |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — captures baseline,
  refreshes localized status, and connects setting changes.
- `src/quillforge/presentation/i18n.py` — adds clean/changed status text.
- `src/quillforge/presentation/theme.py` — adds clean/changed status QSS.
- `scripts/audit_presentation_contracts.py` — guards baseline, signal paths,
  order, equality, accessibility, and QSS.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- The opened `SettingsSnapshot` is the sole baseline; no duplicate dirty model
  or persisted status is introduced.
- Status is explicit text plus semantic state, not color-only feedback.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D307 source status probe | PASS — baseline/equality, preview/behavior updates, locale ordering, semantic states |
| D307 status contrast matrix | PASS — 3 themes × 4 accents; clean 4.87:1, changed 8.79:1 minimum |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `86FF4639FF3D4A6CE5975EC1B0E1BD459ED540C234F23A3C8A7830FE21AE0F3B`; 38,599,151 bytes; source `tree-sha256:6883e2a857339fa0dc863c29fbdd61f756e4db66e44688b818728f10bb8eb6b0` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 166; qwindows 1; Qt6 DLLs 7; QScintilla 1; PyQt runtime hook 1 |

## Unrun checks and reason

Native EXE/Qt startup, screen-reader output, pixel rendering, focus traversal,
DPI, clean-machine behavior, real DLL loading, signing, installer, updater,
registry, and release-owner evidence remain unrun because the active policy
prohibits native software launch and external release actions. Unit tests,
mocks, fixtures, and test harnesses were not created or run.

## Known risks and limits

The source contract proves the intended baseline comparison and dynamic state,
but not native style repaint timing, screen-reader output, or perceived visual
weight. The independent review returned no conclusion after three bounded
waits. The release dossier remains no-go with historical artifact consistency
gates open.

## Acceptance and evidence IDs

- Acceptance: `S347`
- Evidence: `D307-STATUS-PROBE=PASS`, `D307-QSS-MATRIX=PASS`,
  `D307-COMPILEALL=PASS`, `D307-RUFF=PASS`, `D307-FORMAT=PASS`,
  `D307-AUDIT=PASS`, source diagnostics, package identity, PE/archive checks,
  review records, and expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native Settings interaction/screen-reader review and
  artifact-bound startup evidence if full UI or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `86FF4639FF3D4A6CE5975EC1B0E1BD459ED540C234F23A3C8A7830FE21AE0F3B` /
  `38599151` bytes
- Source revision:
  `tree-sha256:6883e2a857339fa0dc863c29fbdd61f756e4db66e44688b818728f10bb8eb6b0`

## Disposition

Accepted with limits. D307 is source-, contract-, contrast-, diagnostic-,
package-, archive-, and handoff-verified. Independent review, native dialog
interaction, clean-machine behavior, and remaining release gates remain open.
