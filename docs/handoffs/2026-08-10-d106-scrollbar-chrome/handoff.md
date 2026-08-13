# Handoff: 2026-08-10-d106-scrollbar-chrome

| Field | Value |
|---|---|
| ID | `2026-08-10-d106-scrollbar-chrome` |
| Delivery / slice | `D106 / UI-54 scrollbar chrome` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:15:00+08:00` |

## User outcome

The editor and other scrollable surfaces now have a symmetric compact
scrollbar presentation. Vertical and horizontal tracks/handles share the
theme tokens and hover state, and the horizontal bar is no longer accidentally
collapsed by the hidden add/sub/page selector group.

## Scope and boundaries

### In scope

- Add explicit horizontal scrollbar track, handle, and hover QSS.
- Remove only `QScrollBar:horizontal` from the hidden subcontrol selector.
- Preserve the existing vertical states and centralized theme ownership.
- Record selector, static, package, traceability, and release-limit evidence.

### Out of scope

- No scroll mode, wrapping, editor setting, widget signal, QScintilla,
  document, operation, persistence, locale, or close policy changed.
- No widget-local stylesheet, new state owner, dependency, asset, or test-only
  file was introduced.
- No QApplication launch, screenshot, native style-engine/DPI visual
  acceptance, clean-machine, cross-machine, signing, installer, updater,
  legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Ramanujan the 3rd / Luna max | Read-only D106 selector consultation; `NO_CONCLUSION` after bounded wait |
| Independent review | Carson the 3rd / Luna max | Read-only QSS review; `NO_CONCLUSION` after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — horizontal scrollbar QSS and
  corrected hidden-subcontrol selector.
- `docs/adr/0133-scrollbar-chrome.md` — decision, invariants, alternatives,
  applicability, review, simplification, and limits.
- `docs/agent-team/reviews/D106-ui-54-scrollbar-parent-review.md` — parent
  five-axis review and simplification assessment.
- `docs/agent-team/reviews/D106-ui-54-scrollbar-independent-review.md` —
  independent `NO_CONCLUSION` record.
- `docs/specs/enterprise-architecture-migration.md`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- `presentation.theme` remains the single QSS token and selector owner.
- The fix is limited to the existing scrollbar block; no scroll policy was
  moved into a widget or application layer.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; selector, source, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 desktop code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A` for this slice.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `D106-SCROLLBAR-SELECTOR-PROBE=PASS` | `PASS` | Horizontal track/handle rules exist and `QScrollBar:horizontal` is absent from the hidden subcontrol group. |
| `uv run python -m compileall -q src/quillforge` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge` | `PASS` | 119 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| `D106-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | SHA `112060A1A3076A14E51D8773F33A5FECA015C86361C24333E96CA606667D84EF`; 38,493,405 bytes; source `tree-sha256:7b738027dfb3263009643e285033378213daf4e95775d19a423568484d706365`. |
| `D106-PACKAGE-NO-LAUNCH-PROBE=PASS` | `PASS` | Packaging completed without launching QuillForge. |

## Unrun checks and reason

- Native Qt style-engine rendering, actual horizontal scrolling, DPI metrics,
  startup, screen-reader output, screenshots, clean-machine, cross-machine,
  hardware, signing, installer, updater, legal, support, and release-owner
  checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS probes cannot prove native style specificity or geometry across
  platform styles and DPI settings.
- Ramanujan and Carson review windows returned `NO_CONCLUSION`; no child PASS
  is claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; the
  known report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S137`, `UI-54-AC01`.
- Evidence: ADR-0133, D106 selector probe, parent/independent review records,
  compile/lint/format checks, package identity, handoff/index/register checks,
  expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded UI or MainWindow/application slice after
  synchronizing D106 traceability and release evidence.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `112060A1A3076A14E51D8773F33A5FECA015C86361C24333E96CA606667D84EF` /
  `38,493,405` bytes.
- Source revision: `tree-sha256:7b738027dfb3263009643e285033378213daf4e95775d19a423568484d706365`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: horizontal scrollbar visibility and visual hierarchy
are corrected centrally, while native rendering and enterprise release gates
remain open.
