# Handoff: 2026-08-11-ui-59-control-affordance

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-59-control-affordance |
| Delivery / slice | UI-59 control-affordance chrome |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T04:25:00+08:00 |

## User outcome

ComboBox dropdowns and SpinBox up/down affordances now have readable,
token-driven base, hover, pressed, and disabled chrome while retaining native
arrow semantics and all settings behavior.

## Scope and boundaries

### In scope

- Style the existing `QComboBox::drop-down` subcontrol.
- Style and explicitly place the existing `QAbstractSpinBox::up-button` and
  `down-button` subcontrols.
- Keep all states derived from centralized theme tokens.

### Out of scope

- No widget constructors, settings value ranges, signals, keyboard traversal,
  accessibility semantics, locale, persistence, dialog layout, or application
  policy changed.
- No custom arrows, image assets, event handlers, or replacement controls.
- No QApplication launch, screenshot, native rendering, DPI/font,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Lorentz the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Banach the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — ComboBox and abstract SpinBox
  subcontrol states.
- `docs/adr/0148-control-affordance-chrome.md`
- `docs/agent-team/reviews/UI-59-control-affordance-parent-review.md`
- `docs/agent-team/reviews/UI-59-control-affordance-independent-review.md`
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- Keep native arrow glyphs and value-changing semantics; QSS owns only the
  subcontrol surface, edge, and state styling.
- The centralized theme is the sole visual token owner; no custom widget or
  second state owner was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| UI-59-CONTROL-AFFORDANCE-PROBE=PASS | PASS | ComboBox and SpinBox subcontrol selectors/placement are present. |
| UI-59-TOKEN-STATE-PROBE=PASS | PASS | Base, hover, pressed, and disabled states use canonical tokens. |
| UI-59-BEHAVIOR-BOUNDARY-PROBE=PASS | PASS | Only centralized theme QSS changed in the source slice. |
| `uv run python -m compileall -q src scripts` | PASS | Static compilation only; no QApplication launch. |
| `uv run ruff check src scripts` | PASS | All checks passed. |
| `uv run ruff format --check src scripts` | PASS | All files are formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Presentation contracts passed after UI-59 source and traceability synchronization; no QApplication launch. |
| UI-59-PACKAGE-IDENTITY-PROBE=PASS | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| UI-59-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no QuillForge process was present afterward. |
| UI-59-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, release manifest, and current dossier bind to UI-59 identity. |
| UI-59-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is UI-59-bound and records the expected no-go decision. |
| UI-59-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the verifier non-zero as required by the evidence boundary. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Native subcontrol rendering, arrow visibility, geometry, focus traversal,
  accessibility, DPI, fonts, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner checks — prohibited or
  outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes prove selector shape and token ownership, not native arrow
  painting, subcontrol geometry, keyboard/screen-reader output, or human
  visual perception.
- Lorentz architecture and Banach independent review both returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; report
  binding and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S152`, `UI-59-AC01`.
- Evidence: ADR-0148, UI-59 source probes, parent/independent review records,
  simplification assessment, static checks, package identity, handoff/index/
  register checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and root `QuillForge.exe`
- SHA-256: B1DFBE8489011D38CCB6C6ACE5F21F94B0D25A63B4E2EEB5AABE898E032760B2
- Size: 38499395 bytes
- Source revision: tree-sha256:5565621062dd0055943a231769f37cc9aa7e78157cc04eb64bd7cae67cc2b870
- Manifest: `dist/QuillForge.release.json`

## Disposition

accepted-with-limits: ComboBox and SpinBox affordances now have clearer
token-driven state hierarchy while native rendering and enterprise release
gates remain open.
