# Handoff: 2026-08-11-d177-settings-guidance-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d177-settings-guidance-hierarchy` |
| Delivery / slice | `D177 / UI-89 / ARCH-164 Settings guidance hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T20:30:00+08:00` |

## User outcome

Settings guidance now belongs visually to the rest of the dialog: the
apply-after-save message and font-fallback message are compact, readable
supporting capsules with distinct accent edges. Settings behavior is
unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` Settings note QSS.
- Apply-note and font-note surfaces, borders, accent edges, radius, text, and
  spacing.
- Static selector, behavior-source, and contrast projection across supported
  themes and accents.

### Out of scope

- No `settings_dialog.py`, `settings_preview.py`, SettingsService, snapshot,
  locale, font fallback, Save/Cancel, persistence, motion, editor, or
  application behavior change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Wegener the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Nash the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized Settings guidance
  hierarchy only.
- D177 ADR, parent/independent review records, and traceability files.

## Decisions and constraints

- `SettingsDialog` and `SettingsPreviewSurface` remain owners of settings
  values, locale, preview, fallback messaging, Save/Cancel, persistence, and
  motion policy; `theme.py` owns only visual rules.
- Existing readable foreground derivation remains authoritative for 砂金,
  paper-sand, and all other theme/accent endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D177-SETTINGS-NOTE-QSS-CONTRACT-PROBE=PASS`.
- `D177-SETTINGS-BEHAVIOR-SOURCE-PROBE=PASS`.
- `D177-SETTINGS-NOTE-CONTRAST-PROBE=PASS` across 3 themes × 4 accents;
  effective minimum was 4.87 for the Paper/Sand violet apply-note text pair.
- `D177-COMPILE-RUFF-FORMAT=PASS`.
- `D177-PACKAGE-BUILD=PASS`.
- `D177-PACKAGE-IDENTITY-PROBE=PASS`.
- `D177-CHECK=PASS`, `D177-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt dialog layout/painting, metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, release-owner, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different note heights, wrapping, or dialog
  spacing than the static projection; runtime visual review remains open.
- Larger user-selected fonts may change note width; native DPI and font
  metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S230`, `D177-AC01`.
- Evidence: ADR-0226, parent/independent review records, D177 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of note wrapping, localized
  widths, font fallback display, screen-reader output, and DPI behavior before
  closing runtime gates.

## Artifact information

The D177 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `EBC815177492FFC9E5ABD5551C428BBDC73D71C9DE72B5F8CB278CBB0814842D`.
- Size: `38550594` bytes.
- Source revision: `tree-sha256:27df2957ea16ed501da5009efb05c49996837d830ad22b3c5f6b9316b6dd740a`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: Settings guidance hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.
