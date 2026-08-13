# Handoff: 2026-08-11-ui-58-dialog-shell

| Field | Value |
|---|---|
| ID | 2026-08-11-ui-58-dialog-shell |
| Delivery / slice | UI-58 dialog-shell edge hierarchy |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-11T03:20:00+08:00 |

## User outcome

Settings and Command Palette now have an intentional token-driven outer frame
and distinct top accent while their existing controls and behavior remain
unchanged.

## Scope and boundaries

### In scope

- Add object-scoped frame/radius styling to `settingsDialog` and
  `commandPalette`.
- Add theme-token top accents that distinguish the two surfaces.
- Preserve existing child QSS, locale, focus, layout, signals, settings
  persistence, and command selection ownership.

### Out of scope

- No dialog constructor, signal, layout, command, settings, plugin, search,
  persistence, locale, or application policy changed.
- No native frame replacement, custom title bar, new widget base class, or new
  visual state owner introduced.
- No QApplication launch, screenshot, native rendering, DPI/font,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Hypatia the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Independent review | Anscombe the 4th / Luna max | `NO_CONCLUSION` after two bounded waits; no child PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — two object-scoped dialog frame/top
  accent contracts.
- `docs/adr/0146-dialog-shell-edge-hierarchy.md`
- `docs/agent-team/reviews/UI-58-dialog-shell-parent-review.md`
- `docs/agent-team/reviews/UI-58-dialog-shell-independent-review.md`
- `docs/specs/enterprise-architecture-migration.md`, `docs/ARCHITECTURE.md`,
  `docs/ROADMAP.md`, `tasks/plan.md`, `tasks/todo.md`.

## Decisions and constraints

- Keep QSS centralized in `theme.py` and scope the new frame contract to the
  two existing semantic dialog object names.
- Preserve child-control selectors as the owners of fields, lists, previews,
  buttons, focus, and action semantics; no new dialog base class or wrapper
  was introduced.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| UI-58-DIALOG-SHELL-PROBE=PASS | PASS | Existing object names and new scoped frame selectors are present. |
| UI-58-TOKEN-ENDPOINT-PROBE=PASS | PASS | Both accents resolve through canonical theme tokens. |
| UI-58-BEHAVIOR-BOUNDARY-PROBE=PASS | PASS | Only theme.py changed in the source slice; dialog source contracts remain unchanged. |
| `uv run python -m compileall -q src scripts` | PASS | Static compilation only; no QApplication launch. |
| `uv run ruff check src scripts` | PASS | All checks passed. |
| `uv run ruff format --check src scripts` | PASS | All files are formatted. |
| `uv run python scripts/audit_presentation_contracts.py` | PASS | Presentation contracts passed after UI-58 source and traceability synchronization; no QApplication launch. |
| UI-58-PACKAGE-IDENTITY-PROBE=PASS | PASS | Root/dist SHA and size match the rebuilt release manifest. |
| UI-58-PACKAGE-NO-LAUNCH-PROBE=PASS | PASS | Package completed without launching QuillForge; no QuillForge process was present afterward. |
| UI-58-JSON-TRACEABILITY-PROBE=PASS | PASS | Acceptance, register, index, release manifest, and current dossier bind to UI-58 identity. |
| UI-58-RELEASE-DOSSIER-PROBE=PASS | PASS | Current dossier is UI-58-bound and records the expected no-go decision. |
| UI-58-RELEASE-EXPECTED-NO-GO=PASS | PASS | Existing open runtime/release gates keep the verifier non-zero as required by the evidence boundary. |
| `scripts\verify_handoff.ps1` | PASS | Handoff schema and traceability checks passed. |
| `scripts\check.ps1` | PASS | Repository formatting, lint, compilation, and static checks passed. |

## Unrun checks and reason

- Native Qt dialog rendering, geometry, focus painting, accessibility, DPI,
  fonts, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static probes prove selector shape and token ownership, not native QSS frame
  painting, actual geometry, focus visibility, or human visual perception.
- Hypatia architecture and Anscombe independent review both returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO; report
  binding and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `S150`, `UI-58-AC01`.
- Evidence: ADR-0146, UI-58 source probes, parent/independent review records,
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
- SHA-256: 7449841EC1FAD1E5F0E7198E25A66212A058EABF60FB8E6ECFC9A13B4199A5FE
- Size: 38497974 bytes
- Source revision: tree-sha256:04077a65cdc384cdddf1774879f133adcb279d3447cb4644138aa4f8837b5270
- Manifest: `dist/QuillForge.release.json`

## Disposition

accepted-with-limits: Settings and Command Palette shell hierarchy is clearer
through centralized token QSS; native rendering and enterprise release gates
remain open.
