# Handoff: 2026-08-13-d305-settings-accessible-names

| Field | Value |
|---|---|
| ID | `2026-08-13-d305-settings-accessible-names` |
| Delivery / slice | `D305 / UI-124 / ARCH-275 Localized Settings accessible names` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-13T10:30:00+08:00` |

## User outcome

Settings controls now receive accessible names from the active localized
labels/text. Switching between English and Simplified Chinese refreshes those
names at the same locale boundary as the visible controls, before the existing
preview refresh.

## Scope and boundaries

In scope: one private Settings accessibility projection helper, its locale
refresh call, an AST-backed contract audit, source diagnostics, package
evidence, and delivery records.

Out of scope: new translation keys, locale values, item data, settings schema,
persistence, signals, keyboard routing, preview semantics, application
ownership, native rendering, screen-reader certification, and release-gate
closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent; Luna/max consultation | Boundary decision, integration, parent review, and handoff |
| Project Manager | parent record | Plan, dependencies, risks, and status |
| Product | parent record | Localized accessibility and inclusive-settings outcome |
| Developer 1 | parent | SettingsDialog accessibility projection |
| Developer 2 | parent | AST-backed presentation contract |
| QA | parent | Source probe, diagnostics, package, archive, and handoff checks |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — projects translated
  labels/text to accessibility names and invokes the projection after locale
  text updates.
- `scripts/audit_presentation_contracts.py` — verifies exact helper structure,
  control membership, setter shape, and ordering.
- ADR, reviews, acceptance, register, architecture, roadmap, plan, todo,
  release handoff, and this handoff record.

## Decisions and constraints

- Existing translation keys remain the source of truth for accessible names.
- The helper is presentation-owned and private; names are not persisted.
- Shared checkout writer: parent only; no worktree or parallel writer was
  used.
- EXE/Qt startup is prohibited by `software_start_allowed=false`; only source,
  static, package, archive, and non-destructive diagnostics are claimed.

## Verification commands and results

| Evidence | Result |
|---|---|
| D305 source probe | PASS — helper loops 2; mapping 9; behavior 3; setters 2; refresh before preview |
| `uv run python -m compileall -q src scripts` | PASS |
| `uv run ruff check src scripts` | PASS |
| `uv run ruff format --check src scripts` | PASS — 149 files formatted |
| `uv run python scripts/audit_presentation_contracts.py` | PASS |
| source `--diagnose-startup` | PASS — composition/restore passed; window shown 0; event loop entered 0 |
| source `--diagnose-file-open README.md` | PASS — startup paths open 1/1; window shown 0; event loop entered 0 |
| package identity | PASS — root/dist SHA `8768782D597C741B9D5D1499E153AAA73AA003D256DE97AADD661353F3A31DA1`; 38,596,329 bytes; source `tree-sha256:b0da9457c7a5e0f6508b2602166017494de9733e3ba9449d6dff0eda6b0c8e30` |
| PE/archive static check | PASS — AMD64 PE32+ Windows GUI; outer entries 166; qwindows 1; Qt6 DLLs 7; QScintilla 1; PyQt runtime hook 1 |

## Unrun checks and reason

Native EXE/Qt startup, screen-reader output, pixel rendering, focus traversal,
DPI, font fallback, clean-machine behavior, real DLL loading, signing,
installer, updater, registry, and release-owner evidence remain unrun because
the active policy prohibits native software launch and external release
actions. Unit tests, mocks, fixtures, and test harnesses were not created or
run.

## Known risks and limits

The source contract proves the intended Qt property values but not native
screen-reader exposure or platform accessibility-tree behavior. Independent
review is PASS for source/contract scope but NO_CONCLUSION for full delivery
signoff because it preceded the new package/handoff records. The release
dossier remains no-go with historical artifact consistency gates open.

## Acceptance and evidence IDs

- Acceptance: `S345`
- Evidence: `D305-PROBE=PASS`, `D305-COMPILEALL=PASS`, `D305-RUFF=PASS`,
  `D305-FORMAT=PASS`, `D305-AUDIT=PASS`, source diagnostics, package identity,
  PE/archive checks, review records, and expected release no-go record.

## Next owner and next action

- Owner: `Project Manager (QuillForge)`
- Action: authorize native visual/focus/screen-reader review and artifact-bound
  startup evidence if accessibility or release acceptance is required.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size:
  `8768782D597C741B9D5D1499E153AAA73AA003D256DE97AADD661353F3A31DA1` /
  `38596329` bytes
- Source revision:
  `tree-sha256:b0da9457c7a5e0f6508b2602166017494de9733e3ba9449d6dff0eda6b0c8e30`

## Disposition

Accepted with limits. D305 is source-, contract-, diagnostic-, package-,
archive-, and handoff-verified. Independent review, native accessibility
behavior, clean-machine behavior, and remaining release gates remain open.
