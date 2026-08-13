# Handoff: 2026-08-11-d179-plugin-catalog-guidance-capsule

| Field | Value |
|---|---|
| ID | `2026-08-11-d179-plugin-catalog-guidance-capsule` |
| Delivery / slice | `D179 / UI-91 / ARCH-166 Plugin catalog guidance capsule` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T21:30:00+08:00` |

## User outcome

The plugin catalog's governance hint now reads as a supporting information
capsule instead of a low-contrast loose footnote. It joins the existing
summary, list, and action hierarchy without changing plugin governance.

## Scope and boundaries

### In scope

- One scoped QSS rule in `src/quillforge/presentation/theme.py` for
  `QDialog#pluginCatalogDialog QLabel#dialogHint`.
- Existing surface, border, pink accent edge, radius, secondary text, and
  spacing tokens.
- Static, package, manifest, handoff, and expected release no-go evidence.

### Out of scope

- No `PluginCatalogDialog`, catalog snapshot, entry formatting, locale,
  selection, approve/revoke signal, trust/approval/execution policy, or
  application behavior change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Cicero the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Pauli the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — catalog hint capsule only.
- D179 ADR, review records, handoff, and traceability files.

## Decisions and constraints

- `PluginCatalogDialog` owns hint content and governance behavior; `theme.py`
  owns visual projection.
- Existing token-derived foregrounds remain authoritative, including the
  砂金/warning path elsewhere in the theme.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D179-PLUGIN-HINT-QSS-CONTRACT-PROBE=PASS`.
- `D179-PLUGIN-HINT-BEHAVIOR-SOURCE-PROBE=PASS`.
- `D179-PLUGIN-HINT-CONTRAST-PROBE=PASS` across 3 themes × 4 accents;
  effective minimum was 4.87.
- `D179-COMPILE-RUFF-FORMAT=PASS`.
- `D179-CHECK=PASS`, `D179-VERIFY-HANDOFF=PASS`.
- `D179-PACKAGE-BUILD=PASS`.
- `D179-PACKAGE-IDENTITY-PROBE=PASS`.
- `D179-MANIFEST-TRACEABILITY-PROBE=PASS`.
- `D179-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`; release verifier remains
  expected `NO-GO` with ten open gates and three mechanical consistency
  failures.

## Unrun checks and reason

- Native Qt dialog layout/painting, metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support, release-owner, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt style/layout engines may vary in hint wrapping and dialog height;
  native visual review remains open.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S232`, `D179-AC01`.
- Evidence: ADR-0228, parent/independent review records, D179 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows review of plugin catalog hint wrapping,
  localized widths, focus/selection, DPI, and screen-reader reading order.

## Artifact information

The D179 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `91628A72591EBC6D7DDAC24742616A9A8972598CCC895AB7E9957523DDE84167`.
- Size: `38549076` bytes.
- Source revision: `tree-sha256:c10a99ac4ac7ea3385ab0266a05fc3f8d70ca7df82d0ba84987c49c29137cd68`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: plugin catalog guidance capsule is delivered; native
rendering, accessibility, and external release evidence remain open.
