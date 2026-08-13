# Handoff: 2026-08-11-d167-settings-typography-preview

| Field | Value |
|---|---|
| ID | `2026-08-11-d167-settings-typography-preview` |
| Delivery / slice | `D167 / UI-79 / ARCH-154 Settings typography preview` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T06:00:00+08:00` |

## User outcome

Settings now previews both typography systems: the interface sample shows the
selected interface font/size, and a second editor sample shows the selected
editor font/size with a localized metadata line.

## Scope and boundaries

### In scope

- `settings_preview.py` editor sample, metadata, and QFont projection.
- `settings_dialog.py` editor font/size preview refresh wiring.
- `i18n.py` English/Chinese editor preview keys.
- `theme.py` object-scoped preview surface tokens.

### Out of scope

- No SettingsSnapshot/SettingsService, font allowlist, UserRole,
  persistence, post-save application, editor adapter, locale architecture,
  theme policy, size range, motion, or file-open behavior change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Ramanujan the 5th / Luna max | `PASS` after QSS/font interpolation correction |
| Independent review | Franklin the 5th / Luna max | `PASS` after final correction; no Git baseline for full-diff proof |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/settings_preview.py` — both QFont samples and
  editor metadata.
- `src/quillforge/presentation/settings_dialog.py` — editor font/size refresh
  connections and projection values.
- `src/quillforge/presentation/i18n.py` — editor preview keys in both bundles.
- `src/quillforge/presentation/theme.py` — scoped editor-sample/meta tokens;
  removed preview font interpolation parameters.
- `docs/adr/0216-settings-typography-preview.md`.
- D167 parent/independent review records and traceability files.

## Decisions and constraints

- `SettingsPreviewSurface` remains a one-way presentation projection; it does
  not save, apply, or mutate settings.
- `QFont` owns typography projection; `preview_stylesheet()` owns only visual
  tokens and has no dynamic font inputs.
- The parent is the sole shared-checkout writer. No Git/worktree operation
  was used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D167-AST-PROBE=PASS`.
- `D167-EDITOR-FONT-SIZE-PROJECTION-PROBE=PASS`.
- `D167-REFRESH-WIRING-PROBE=PASS`.
- `D167-SCOPED-QSS-PROBE=PASS`.
- `D167-I18N-KEY-PROBE=PASS`.
- `D167-I18N-PLACEHOLDER-PROBE=PASS`.
- `D167-QFONT-BOTH-SAMPLES-PROBE=PASS`.
- `D167-SAFE-PREVIEW-QSS-PROBE=PASS`.
- `D167-COMPILEALL=PASS`.
- `D167-RUFF=PASS`.
- `D167-FORMAT=PASS`.
- `D167-PACKAGE-BUILD=PASS`.
- `D167-PACKAGE-IDENTITY-PROBE=PASS`.
- `D167-CHECK=PASS`, `D167-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Settings preview rendering, installed-font fallback metrics, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native style engines may apply widget font precedence differently from
  static source expectations; runtime popup and preview review is still open.
- Unavailable installed fonts may fall back to system fonts; static evidence
  does not prove fallback metrics at every DPI.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S220`, `D167-AC01`.
- Evidence: ADR-0216, parent/independent review records, D167 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize a native Settings preview review on the target Windows
  environment, capture installed-font/DPI/accessibility evidence, and keep
  release gates open until their authoritative evidence exists.

## Artifact information

The D167 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `86BA71B9704FDAFE3FFAC2A763C74C79CCF6502AA037504BBDC5B6B867D38ED9`.
- Size: `38547061` bytes.
- Source revision: `tree-sha256:c7d828d5fd5077d249a0f8997f137220493f156b31680f3b0a3f7155094443da`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: editor typography preview is delivered; native
rendering, fallback metrics, accessibility, and external release evidence
remain open.

