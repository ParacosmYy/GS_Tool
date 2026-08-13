# Handoff: 2026-08-11-ui-57-workspace-dock

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-57-workspace-dock |
| Delivery / slice | UI-57 / ARCH-88 workspace-dock chrome hierarchy |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T02:15:00+08:00 |

## User outcome

The workspace dock now has a token-driven frame, title, close, and float
chrome hierarchy while native docking affordances and workspace behavior stay
unchanged.

## Scope and boundaries

### In scope

- Scope the existing dock chrome to WorkspaceDock.
- Add readable title and frame styling.
- Add close/float hover, pressed, and disabled states through native QSS
  subcontrols.
- Record public-source applicability, parent review, independent review,
  simplification, static, package, and release-limit evidence.

### Out of scope

- No dock placement/floating/closing flags, native icons, panel signals,
  workspace navigation, locale behavior, loading/error policy, persistence,
  or MainWindow ownership changed.
- No custom title bar, new widget, new icon system, global QDockWidget style,
  or test-only asset was introduced.
- No QApplication launch, native rendering, docking interaction, screenshot,
  screen-reader, DPI, font, clean-machine, cross-machine, signing,
  installer, updater, legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Lagrange the 4th / Luna max | Read-only UI-57 chrome consultation; NO_CONCLUSION after bounded wait |
| Independent review | Plato the 4th / Luna max | Read-only UI-57 review; NO_CONCLUSION after bounded wait |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- src/quillforge/presentation/theme.py — WorkspaceDock-scoped frame/title/
  close/float QSS.
- src/quillforge/presentation/workspace_surface.py — existing WorkspaceDock
  identity retained as the styling boundary.
- docs/adr/0144-workspace-dock-chrome-hierarchy.md — decision, invariants,
  alternatives, applicability, review, simplification, and limits.
- docs/agent-team/reviews/UI-57-workspace-dock-parent-review.md — parent
  review.
- docs/agent-team/reviews/UI-57-workspace-dock-independent-review.md —
  independent NO_CONCLUSION record.
- docs/specs/enterprise-architecture-migration.md, docs/ARCHITECTURE.md,
  docs/ROADMAP.md, tasks/plan.md, tasks/todo.md.

## Decisions and constraints

- Native QDockWidget subcontrols remain the only close/float affordance; QSS
  owns appearance, not docking policy.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and
  non-launching static evidence are the permitted validation boundary.
- This is Python/PyQt6 presentation code. Embedded C/C++ assurance and vendor
  manufacturer requirements are N/A.
- Public CloudWeGo material remains an engineering reference only; no private
  ByteDance standard, certification, or compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| UI-57-WORKSPACE-DOCK-PROBE=PASS | PASS | Existing WorkspaceDock identity, scoped selectors, native subcontrols, and interaction states are present. |
| UI-57-TOKEN-CONTRAST-PROBE=PASS | PASS | Title foreground passes the static 4.5 contrast threshold for three themes and four accent choices. |
| uv run python -m compileall -q src scripts | PASS | Static compilation only; no QApplication launch. |
| uv run ruff check src scripts | PASS | All checks passed. |
| uv run ruff format --check src scripts | PASS | All files are formatted. |
| uv run python scripts/audit_presentation_contracts.py | PASS | Existing presentation contract/error/observability gate. |
| UI-57-PACKAGE-IDENTITY-PROBE=PASS | PASS | 56808A2199173BA20CF54D5625FD1C40446D5ECC8613E54E8E1236A8DBA27696; 38,498,738 bytes; source tree-sha256:ea2f2cc01b7456a6a7d2c707a00a6b582b3638edca01209ce18a9d5815c91c7e. |
| UI-57-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no process was started by the package command. |
| UI-57-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, and release manifest are synchronized to UI-57 identity. |
| UI-57-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is UI-57-bound and intentionally no-go. |
| UI-57-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the release verifier intentionally non-zero. |
| scripts\verify_handoff.ps1 | PASS | Final handoff/index contract. |
| scripts\check.ps1 | PASS | Repository/static checks after traceability updates. |

## Unrun checks and reason

- Native QDockWidget painting/positioning, docking/floating interaction,
  QApplication startup, screen-reader output, DPI/font metrics,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static selector probes prove authored hierarchy, not native QDockWidget
  subcontrol painting or actual docking interaction.
- Lagrange architecture and Plato independent review returned NO_CONCLUSION; no
  child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: S148, UI-57-AC01.
- Evidence: ADR-0144, UI-57 dock/contrast probes, parent/independent review
  records, compile/lint/format checks, package identity, handoff/index/register
  checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded visual or MainWindow/application contract
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: dist/QuillForge.exe and root QuillForge.exe
- SHA-256: 56808A2199173BA20CF54D5625FD1C40446D5ECC8613E54E8E1236A8DBA27696
- Size: 38,498,738 bytes
- Source revision: tree-sha256:ea2f2cc01b7456a6a7d2c707a00a6b582b3638edca01209ce18a9d5815c91c7e
- Manifest: dist/QuillForge.release.json

## Disposition

accepted-with-limits: workspace dock chrome is improved through scoped native
QSS subcontrols while native rendering, docking interaction, runtime, and
enterprise release gates remain open.
