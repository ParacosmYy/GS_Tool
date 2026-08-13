# Handoff: 2026-08-11-ui-62-tab-close-affordance

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-62-tab-close-affordance |
| Delivery / slice | UI-62 document tab close-affordance hierarchy |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T05:55:00+08:00 |

## User outcome

Document-tab close affordances now have a clear minimum target size and
explicit focus, pressed, hover, and disabled visual states while preserving
the existing close signal and document lifecycle.

## Scope and boundaries

### In scope

- Add 18px minimum width/height to the existing document-tab close subcontrol.
- Add token-driven focus and disabled states.
- Preserve existing danger-colored hover and pressed states.
- Keep the selector scoped to `QTabBar#documentTabBar`.

### Out of scope

- No `DocumentTabSurface` behavior, tab lifecycle, close confirmation,
  document policy, keyboard routing, locale, theme token, or persistence
  contract changed.
- No custom close widget, delegate, icon asset, or test-only asset.
- No QApplication launch, screenshot, native rendering, DPI/font,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Ampere the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Sagan the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — scoped close-subcontrol QSS.
- `docs/adr/0151-tab-close-affordance-hierarchy.md`
- `docs/agent-team/reviews/UI-62-tab-close-affordance-parent-review.md`
- `docs/agent-team/reviews/UI-62-tab-close-affordance-independent-review.md`
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- The centralized theme stylesheet owns the close affordance states; the
  tab surface remains the owner of tab composition and signal projection.
- The existing danger hover/pressed states remain the destructive cue; focus
  is an alternate-accent keyboard cue and disabled is muted.
- The parent is the sole shared-checkout writer. No Git/worktree operation
  was used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `UI-62-TAB-CLOSE-AFFORDANCE-PROBE=PASS` | PASS | Scoped base/focus/pressed/disabled selector and minimum-size coverage. |
| `UI-62-TAB-CLOSE-TOKEN-PROBE=PASS` | PASS | New states use centralized surface, accent, danger, and border tokens. |
| `UI-62-BEHAVIOR-BOUNDARY-PROBE=PASS` | PASS | Existing tab-close signal and authored tab-bar identities remain present. |
| `uv run python -m compileall -q src/quillforge/presentation/theme.py src/quillforge/presentation/document_tab_surface.py` | PASS | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge/presentation/theme.py src/quillforge/presentation/document_tab_surface.py` | PASS | All checks passed. |
| `uv run ruff format --check src scripts` | PASS | Recorded after full repository formatting. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Presentation contract audit passed; no QApplication launch. |
| `UI-62-PACKAGE-IDENTITY-PROBE=PASS` | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| `UI-62-PACKAGE-NO-LAUNCH-PROBE=PASS` | PASS | Packaging completed without launching QuillForge; no process afterward. |
| `UI-62-JSON-TRACEABILITY-PROBE=PASS` | PASS | Acceptance, register, index, manifest, and dossier bind to UI-62 identity. |
| `UI-62-RELEASE-DOSSIER-PROBE=PASS` | PASS | Current dossier is UI-62-bound and records the expected no-go decision. |
| `UI-62-RELEASE-EXPECTED-NO-GO=PASS` | PASS | Open runtime/release gates keep the verifier non-zero as required. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Native tab-close rendering, keyboard focus output, accessibility output,
  DPI, fonts, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes prove selector/token wiring, not native QSS subcontrol
  behavior, actual hit geometry, screen-reader output, or visual perception.
- Ampere architecture and Sagan independent review both returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; report
  binding and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S155`, `UI-62-AC01`.
- Evidence: ADR-0151, UI-62 source probes, parent/independent review records,
  simplification assessment, static checks, package identity, handoff/index/
  register checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: 2D8086E4C5BEA0222B6CF622EE9A6439CB775F89F205359B097846A19089AA50
- Size: 38502031 bytes
- Source revision: tree-sha256:c45c417397d296d8113291c01ed51742b15910bc287cac9227794aa3eca55b98
- Manifest: `dist/QuillForge.release.json`

## Disposition

accepted-with-limits: document-tab close affordances now have explicit scoped
state hierarchy while tab behavior, native rendering evidence, and enterprise
release gates remain open.
