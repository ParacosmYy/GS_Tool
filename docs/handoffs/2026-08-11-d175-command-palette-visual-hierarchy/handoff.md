# Handoff: 2026-08-11-d175-command-palette-visual-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d175-command-palette-visual-hierarchy` |
| Delivery / slice | `D175 / UI-87 / ARCH-162 Command Palette visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T18:30:00+08:00` |

## User outcome

The Command Palette now exposes a clearer scan path: the result list has an
explicit focus boundary, hover feedback, and selected-hover treatment, while
the existing keyboard hint reads as a compact supporting capsule. Command
filtering and activation behavior are unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` Command Palette QSS.
- Result-list focus/hover/selected-hover hierarchy and hint presentation.
- Static selector, behavior-source, and contrast projection across supported
  themes and accents.

### Out of scope

- No `command_palette.py` or `command_palette_surface.py` behavior change.
- No command registry, filtering, ordering, stable ID, return-key, item
  activation, locale, modal, application, or persistence change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Herschel the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Euclid the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized Command Palette visual
  hierarchy only.
- D175 ADR, parent/independent review records, and traceability files.

## Decisions and constraints

- `CommandPaletteDialog` remains the owner of query filtering, current-row
  selection, stable-ID projection, item activation, return-key acceptance,
  locale, and modal lifecycle; `theme.py` owns only visual rules.
- Existing readable foreground derivation remains authoritative for 砂金,
  paper-sand, and all other theme/accent endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D175-QSS-CONTRACT-PROBE=PASS`.
- `D175-COMMAND-BEHAVIOR-SOURCE-PROBE=PASS`.
- `D175-COMMAND-PALETTE-CONTRAST-PROBE=PASS` across 3 themes × 4 accents;
  effective minimum was 4.87 for the Paper/Sand violet hint-text pair.
- `D175-COMPILE-RUFF-FORMAT=PASS`.
- `D175-PACKAGE-BUILD=PASS`.
- `D175-PACKAGE-IDENTITY-PROBE=PASS`.
- `D175-CHECK=PASS`, `D175-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt list painting/focus/layout, metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, release-owner, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different list focus, item hover, or hint height
  than the static projection; runtime visual review remains open.
- Larger user-selected fonts may change item wrapping and hint width; native
  DPI and font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S228`, `D175-AC01`.
- Evidence: ADR-0224, parent/independent review records, D175 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of list focus, hover/selected
  transitions, localized hint widths, screen-reader output, and DPI behavior
  before closing runtime gates.

## Artifact information

The D175 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `93A85B6A7DD303AD53F89EBC5B71C66198B586F7AF7BBE2294687B36415519E2`.
- Size: `38549974` bytes.
- Source revision: `tree-sha256:3ce23c2c9031e1c7bf23889f2f07e6bd3cd7b3d15bf71f1f51e3bd753a28b1d7`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: Command Palette visual hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.
