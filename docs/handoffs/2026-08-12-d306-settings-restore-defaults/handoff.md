# Handoff: 2026-08-12-d306-settings-restore-defaults

| Field | Value |
|---|---|
| ID | `2026-08-12-d306-settings-restore-defaults` |
| Delivery / slice | `D306 / UI-125 / ARCH-276 Settings draft restore defaults` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-12T22:00:00+08:00` |

## User outcome

Settings now has a visible “Restore defaults” action. It resets the current
draft for language, theme, accent, fonts, sizes, styles, wrapping, line
numbers, and motion in one coherent operation. The user can inspect the
preview, click Save to persist, or click Cancel to discard the draft.

## Scope and boundaries

In scope: one RestoreDefaults action, 12-control draft projection, localized
tooltip/accessibility name, warning-tone QSS, static contract, diagnostics,
package evidence, and delivery records.

Out of scope: new settings schema/data, SettingsService changes, persistence
outside Save, MainWindow/application projection, locale architecture, new
animation policy, native screen-reader certification, and release-gate closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Reversible settings recovery outcome |
| Developer 1 | parent | SettingsDialog RestoreDefaults projection |
| Developer 2 | parent | i18n/theme/static contract integration |
| QA | parent | Source probe, matrix, diagnostics, package, archive, and handoff |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — adds the RestoreDefaults
  action and signal-blocked draft reset.
- `src/quillforge/presentation/i18n.py` — adds English/Chinese action and
  tooltip strings.
- `src/quillforge/presentation/theme.py` — adds warning-tone reset action
  states using existing tokens.
- `scripts/audit_presentation_contracts.py` — guards reset role, 12 controls,
  draft-only boundary, order, and QSS states.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- `DEFAULT_SETTINGS` remains the sole default source; no duplicate defaults are
  introduced.
- The reset action never calls Save, `accept()`, `reject()`, or a persistence
  service.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D306 source reset probe | PASS — 12 controls, default snapshot, signal blocking, draft-only, locale order, QSS |
| D306 reset contrast matrix | PASS — 3 themes × 4 accents; state minima 4.87:1 normal, 8.79:1 hover/focus, 8.64:1 pressed, 5.14:1 disabled |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `D8D287F7D15BB2F599742DB9299D49DDCB7171B754180BD3A4B6237188EA092F`; 38,598,403 bytes; source `tree-sha256:752385e2237d2698ea9080d60ab04df6ddb0959ade1fc3b36082f954163a3ad3` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 166; qwindows 1; Qt6 DLLs 7; QScintilla 1; PyQt runtime hook 1 |

## Unrun checks and reason

Native EXE/Qt startup, screen-reader output, pixel rendering, focus traversal,
DPI, clean-machine behavior, real DLL loading, signing, installer, updater,
registry, and release-owner evidence remain unrun because the active policy
prohibits native software launch and external release actions. Unit tests,
mocks, fixtures, and test harnesses were not created or run.

## Known risks and limits

The source contract proves draft-only control flow but not native button order,
platform reset semantics, screen-reader output, or perceived visual weight.
The independent review returned no conclusion after three bounded waits. The
release dossier remains no-go with historical artifact consistency gates open.

## Acceptance and evidence IDs

- Acceptance: `S346`
- Evidence: `D306-RESET-PROBE=PASS`, `D306-QSS-MATRIX=PASS`,
  `D306-COMPILEALL=PASS`, `D306-RUFF=PASS`, `D306-FORMAT=PASS`,
  `D306-AUDIT=PASS`, source diagnostics, package identity, PE/archive checks,
  review records, and expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native Settings interaction/screen-reader review and
  artifact-bound startup evidence if full UI or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `D8D287F7D15BB2F599742DB9299D49DDCB7171B754180BD3A4B6237188EA092F` /
  `38598403` bytes
- Source revision:
  `tree-sha256:752385e2237d2698ea9080d60ab04df6ddb0959ade1fc3b36082f954163a3ad3`

## Disposition

Accepted with limits. D306 is source-, contract-, contrast-, diagnostic-,
package-, archive-, and handoff-verified. Independent review, native dialog
interaction, clean-machine behavior, and remaining release gates remain open.
