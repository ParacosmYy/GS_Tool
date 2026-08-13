# Handoff: 2026-08-11-d171-document-tab-visual-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d171-document-tab-visual-hierarchy` |
| Delivery / slice | `D171 / UI-83 / ARCH-158 Document-tab visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T14:30:00+08:00` |

## User outcome

The document tab rail now has one restrained container, flat inactive tabs,
clear hover feedback, and a more intentional selected tab. Focus, disabled,
and close-button states remain visible and readable. Document behavior and
dirty-state semantics are unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` document-tab and close-button QSS.
- Outer rail, inactive, hover, selected, focus, disabled, and close target
  visual hierarchy.
- Static selector and contrast projection across supported themes and accents.

### Out of scope

- No `document_tab_surface.py`, tab creation/removal, title, modified state,
  icon contract, signal, shortcut, locale, domain, application, or persistence
  change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | McClintock the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Descartes the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized document-tab visual
  hierarchy only.
- `docs/adr/0220-document-tab-visual-hierarchy.md`.
- D171 parent/independent review records and traceability files.

## Decisions and constraints

- `DocumentTabSurface` remains the sole owner of tab projection behavior;
  `theme.py` remains the sole owner of these visual rules.
- Existing foreground and accent tokens remain authoritative for all supported
  themes, including 砂金/paper-sand endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D171-COMPILEALL=PASS`.
- `D171-RUFF=PASS` and `D171-FORMAT=PASS`.
- `D171-QSS-CONTRACT-PROBE=PASS`.
- `D171-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective minimum
  was 3.65 for the bounded tab-state projection.
- `D171-PACKAGE-BUILD=PASS`.
- `D171-PACKAGE-IDENTITY-PROBE=PASS`.
- `D171-CHECK=PASS`, `D171-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt painting/layout, tab metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different tab text/icon/close spacing than the
  static source projection; runtime visual review remains open.
- Larger user-selected fonts may change the final tab width and elision;
  native DPI and font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S224`, `D171-AC01`.
- Evidence: ADR-0220, parent/independent review records, D171 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of tab focus, close targets,
  long-title elision, and large-font behavior before closing runtime gates.

## Artifact information

The D171 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `862C7FE18EDC82E73F3CD5A9768566D41EEEA313FD1F89B3BF58F3759865B242`.
- Size: `38550744` bytes.
- Source revision: `tree-sha256:0c2b2ca38eaf6f596c356aac5790487d5afe610d3f94dd827e08876cb0b35363`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: document-tab visual hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.
