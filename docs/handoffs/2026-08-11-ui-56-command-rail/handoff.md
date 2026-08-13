# Handoff: 2026-08-11-ui-56-command-rail

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-56-command-rail |
| Delivery / slice | UI-56 / ARCH-87 command-rail visual role hierarchy |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T01:45:00+08:00 |

## User outcome

The top command rail now distinguishes the primary Save action, quiet utility
actions, and workspace context while retaining the existing standard actions.
The role hierarchy is theme-token driven and presentation-only; command
callbacks, shortcuts, menus, locale, icons, document/workspace policy, and
close behavior remain unchanged.

## Scope and boundaries

### In scope

- Add the closed ToolbarActionRole contract with a standard default.
- Project role metadata to stable commandBar tool-button properties.
- Add scoped primary/quiet/context QSS with explicit interaction states.
- Assign roles at the MainWindow toolbar composition site.
- Record public-source applicability, parent review, independent review,
  simplification, static, package, and release-limit evidence.

### Out of scope

- No CommandRegistry, QAction callback, shortcut, menu, locale, icon,
  toolbar order/layout, TaskRunner, document, workspace, persistence, or close
  policy changed.
- No global QToolButton styling, theme token model, new widget, animation
  subsystem, or test-only asset was introduced.
- No QApplication launch, native rendering, screenshot, screen-reader, DPI,
  font, clean-machine, cross-machine, signing, installer, updater, legal,
  support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Wegener the 4th / Luna max | Read-only UI-56 visual/architecture consultation; NO_CONCLUSION after bounded wait |
| Independent review | Pauli the 4th / Luna max | Read-only UI-56 review; NO_CONCLUSION after two bounded waits |
| Parent | Architect | Sole writer, integration, source review, simplification, packaging, and verification |

No child PASS is claimed.

## Changed files and modules

- src/quillforge/presentation/command_surface.py — typed role metadata and
  QToolButton property projection.
- src/quillforge/presentation/main_window.py — explicit role assignment for
  toolbar actions.
- src/quillforge/presentation/theme.py — commandBar-scoped role hierarchy QSS.
- docs/adr/0143-command-rail-visual-role-hierarchy.md — decision, invariants,
  alternatives, applicability, review, simplification, and limits.
- docs/agent-team/reviews/UI-56-command-rail-parent-review.md — parent review.
- docs/agent-team/reviews/UI-56-command-rail-independent-review.md —
  independent review record.
- docs/specs/enterprise-architecture-migration.md, docs/ARCHITECTURE.md,
  docs/ROADMAP.md, tasks/plan.md, tasks/todo.md.

## Decisions and constraints

- Role metadata is a closed presentation contract; command policy remains in
  the existing application layer.
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
| UI-56-COMMAND-ROLE-PROBE=PASS | PASS | Closed role contract, backward-compatible default, role coverage, commandRole projection, and composition-site assignments. |
| UI-56-TOKEN-CONTRAST-PROBE=PASS | PASS | Primary and hover foreground endpoints pass the static 4.5 contrast threshold for three themes and four accent choices. |
| uv run python -m compileall -q src scripts | PASS | Static compilation only; no QApplication launch. |
| uv run ruff check src scripts | PASS | All checks passed. |
| uv run ruff format --check src scripts | PASS | All files are formatted. |
| uv run python scripts/audit_presentation_contracts.py | PASS | Existing presentation contract/error/observability gate. |
| UI-56-PACKAGE-IDENTITY-PROBE=PASS | PASS | AF0395B69AE843C0467452B256BD3B8E78FCC06C490F9FFAAFB24AEBA6B42BB4; 38,498,238 bytes; source tree-sha256:c6999291f41cc8e0dec50b993313f1648bf2d8c38f5ffc9b17b34810e6f9b2b5. |
| UI-56-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no process was started by the package command. |
| UI-56-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, and release manifest are synchronized to UI-56 identity. |
| UI-56-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is UI-56-bound and intentionally no-go. |
| UI-56-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the release verifier intentionally non-zero. |
| scripts\verify_handoff.ps1 | PASS | Final handoff/index contract. |
| scripts\check.ps1 | PASS | Repository/static checks after traceability updates. |

## Unrun checks and reason

- Native Qt QSS painting, actual human visual perception, QApplication startup,
  screen-reader output, DPI/font metrics, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static token probes prove authored contrast choices, not native QSS painting
  or visual perception on every environment.
- Wegener architecture and Pauli independent review both returned
  NO_CONCLUSION; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; known
  report-binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: S147, UI-56-AC01.
- Evidence: ADR-0143, UI-56 role/contrast probes, parent/independent review
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
- SHA-256: AF0395B69AE843C0467452B256BD3B8E78FCC06C490F9FFAAFB24AEBA6B42BB4
- Size: 38,498,238 bytes
- Source revision: tree-sha256:c6999291f41cc8e0dec50b993313f1648bf2d8c38f5ffc9b17b34810e6f9b2b5
- Manifest: dist/QuillForge.release.json

## Disposition

accepted-with-limits: command-rail visual hierarchy is improved through closed
presentation roles and scoped token QSS, while native rendering, runtime, and
enterprise release gates remain open.
