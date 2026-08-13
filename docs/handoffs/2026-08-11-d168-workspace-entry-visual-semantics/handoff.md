# Handoff: 2026-08-11-d168-workspace-entry-visual-semantics

| Field | Value |
|---|---|
| ID | `2026-08-11-d168-workspace-entry-visual-semantics` |
| Delivery / slice | `D168 / UI-80 / ARCH-155 Workspace entry visual semantics` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T07:00:00+08:00` |

## User outcome

Workspace entries are easier to scan: files, folders, and unavailable items
retain distinct authored icon semantics, file hints explain first-click open,
folder hints explain double-click navigation, and provider diagnostics remain
visible when access fails. The existing file-open fix and folder navigation
behavior are unchanged.

## Scope and boundaries

### In scope

- `workspace_panel.py` semantic icon foreground selection and localized hint
  refresh.
- `i18n.py` English/Chinese file, directory, and unavailable-item hints.
- Static palette projection across all supported theme/accent combinations.

### Out of scope

- No domain model, WorkspaceService, MainWindow, containment, busy guard,
  duplicate-tab, async opening, file/folder signal, theme token, QSS, or
  custom delegate change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Dalton the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Socrates the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — palette-aware file icon
  emphasis, folder/inaccessible retention, localized hint projection, and
  locale/theme refresh wiring.
- `src/quillforge/presentation/i18n.py` — English/Chinese entry hints.
- `docs/adr/0217-workspace-entry-visual-semantics.md`.
- D168 parent/independent review records and traceability files.

## Decisions and constraints

- Reuse existing Qt palette roles; no new theme token or global selector was
  introduced.
- Keep the Link role for file icon emphasis on dark canvases, with a primary
  text fallback on light canvases so the existing pressed-row surface remains
  readable.
- Provider error text always wins over the generic unavailable hint.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D168-AST-PROBE=PASS`.
- `D168-I18N-SEMANTIC-HINT-PROBE=PASS`.
- `D168-TOOLTIP-PRECEDENCE-PROBE=PASS`.
- `D168-ICON-REFRESH-PROBE=PASS`.
- `D168-SIGNAL-PRESERVATION-PROBE=PASS`.
- `D168-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective minimum
  was at least 3.0 for file, selected/pressed, folder, and disabled icon
  surfaces.
- `D168-COMPILEALL=PASS`.
- `D168-RUFF=PASS` and `D168-FORMAT=PASS`.
- `D168-PACKAGE-BUILD=PASS`.
- `D168-PACKAGE-IDENTITY-PROBE=PASS`.
- `D168-CHECK=PASS`, `D168-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native icon rendering, tooltip timing, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt style engines may render authored icon alpha/detail differently
  from the source projection; runtime visual review remains open.
- A provider diagnostic is intentionally preserved as returned; this slice does
  not translate filesystem error text.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S221`, `D168-AC01`.
- Evidence: ADR-0217, parent/independent review records, D168 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize a native Windows visual/accessibility review of workspace
  rows and keep release gates open until authoritative evidence exists.

## Artifact information

The D168 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `93B96E67A56C9F0E1F5BCC8A2F7CE1E58BB07743C21AFDCF7A91FD045A2D1934`.
- Size: `38549274` bytes.
- Source revision: `tree-sha256:fcfbd2a4b51f67f7812095019e35fc27b52625bd07b6bff4ddd589aa75fd3c83`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: workspace entry scanability is delivered; native
rendering, accessibility, and external release evidence remain open.
