# Handoff: 2026-08-11-d169-findbar-authored-action-icons

| Field | Value |
|---|---|
| ID | `2026-08-11-d169-findbar-authored-action-icons` |
| Delivery / slice | `D169 / UI-81 / ARCH-156 FindBar authored action icons` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T07:30:00+08:00` |

## User outcome

Find/Replace actions now have compact authored icons beside their existing
localized labels: up/down navigation, replace, cancel, and close are visually
scannable and consistent with the command rail and workspace surfaces. Search
signals, keyboard shortcuts, query state, busy cancellation, and primary-action
semantics are unchanged.

## Scope and boundaries

### In scope

- `find_bar.py` icon projection and current-palette refresh.
- `icon_contract.py` pure `ARROW_DOWN` semantic key.
- `icons.py` authored down-arrow painter branch.
- Static icon contrast projection for all supported theme/accent combinations.

### Out of scope

- No FindSurface/MainWindow/application/domain/service/coordinator change.
- No query/replacement/case behavior, signal, shortcut, visibility,
  operation-active, cancel, primary-action, theme token, QSS, or runtime
  policy change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Heisenberg the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Carson the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/find_bar.py` — action icon projection and
  locale/theme refresh.
- `src/quillforge/presentation/icon_contract.py` — pure `ARROW_DOWN` key.
- `src/quillforge/presentation/icons.py` — down-arrow vector painting.
- `docs/adr/0218-findbar-authored-action-icons.md`.
- D169 parent/independent review records and traceability files.

## Decisions and constraints

- FindBar remains the only owner of its button projection; the existing
  editor-shell locale refresh is the theme/accent refresh seam.
- Dark canvases retain accent-alt icon details. Light canvases use primary
  button text for secondary strokes so pressed surfaces remain readable.
- Disabled icons use the existing `QIcon.Mode.Disabled` pixmap path.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D169-CONTRACT-AST-PROBE=PASS`.
- `D169-ICONS-AST-PROBE=PASS`.
- `D169-FIND-AST-PROBE=PASS`.
- `D169-ICON-MAPPING-PROBE=PASS`.
- `D169-SIGNAL-PRESERVATION-PROBE=PASS`.
- `D169-LOCALE-REFRESH-PROBE=PASS`.
- `D169-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective minimum
  was 5.14 for normal, hover, pressed, and disabled projections.
- `D169-COMPILEALL=PASS`.
- `D169-RUFF=PASS` and `D169-FORMAT=PASS`.
- `D169-PACKAGE-BUILD=PASS`.
- `D169-PACKAGE-IDENTITY-PROBE=PASS`.
- `D169-CHECK=PASS`, `D169-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native icon painting, button metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different text/icon spacing than the static
  source projection; runtime visual review remains open.
- A compact icon beside a localized label can affect minimum width at larger
  font settings; native DPI/font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S222`, `D169-AC01`.
- Evidence: ADR-0218, parent/independent review records, D169 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize a native Windows review of FindBar text/icon spacing and
  accessibility, then keep release gates open until authoritative evidence
  exists.

## Artifact information

The D169 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `0ACF6C73069EC038FDF2DF140E6A7B0F5E2D1DBAB70AE049732852C1F63DDC46`.
- Size: `38550610` bytes.
- Source revision: `tree-sha256:52afafa55e5e205794b1977e9c7fa84893f762c9b4f40b70bac9642249bc7bf1`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: FindBar action affordance is delivered; native
rendering, accessibility, and external release evidence remain open.
