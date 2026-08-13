# Handoff: 2026-08-11-d172-editor-stage-visual-depth

| Field | Value |
|---|---|
| ID | `2026-08-11-d172-editor-stage-visual-depth` |
| Delivery / slice | `D172 / UI-84 / ARCH-159 Editor-stage visual depth` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T15:30:00+08:00` |

## User outcome

The central editor now reads as one primary canvas inside a softer stage
surface instead of two equally heavy frames. Focus and selection emphasis are
preserved, while fonts, syntax, caret, line numbers, wrapping, and editing
behavior remain unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` editor-shell/editor QSS.
- Stage surface, border weight, canvas radius, focus, and selection contract.
- Static selector and editor palette contrast projection across themes/accents.

### Out of scope

- No `editor_widget.py`, `editor_shell_surface.py`, lexer, font, syntax,
  selection, caret, line-number, wrapping, document, signal, or application
  policy change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Peirce the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Galileo the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized editor-stage visual
  hierarchy only.
- `docs/adr/0221-editor-stage-visual-depth.md`.
- D172 parent/independent review records and traceability files.

## Decisions and constraints

- `EditorWidget` remains the sole owner of QScintilla behavior and palette
  application; `theme.py` owns only the shell stylesheet projection.
- Existing selection and focus tokens remain authoritative, including 砂金
  and paper-sand endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D172-COMPILEALL=PASS`.
- `D172-RUFF=PASS` and `D172-FORMAT=PASS`.
- `D172-QSS-CONTRACT-PROBE=PASS`.
- `D172-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective minimum
  was 4.55 for shell/editor/focus/selection/syntax projections.
- `D172-PACKAGE-BUILD=PASS`.
- `D172-PACKAGE-IDENTITY-PROBE=PASS`.
- `D172-CHECK=PASS`, `D172-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native QScintilla painting/layout, editor metrics, screen-reader output, DPI,
  accessibility, clean-machine, cross-machine, signing, installer, updater,
  legal, support-owner acceptance, and runtime startup — prohibited or require
  an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt/QScintilla styles may produce different editor margins and radius
  clipping than the static projection; runtime visual review remains open.
- Larger selected fonts can change the canvas/tab relationship; native DPI and
  font metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates are
  open.

## Acceptance and evidence IDs

- Acceptance: `S225`, `D172-AC01`.
- Evidence: ADR-0221, parent/independent review records, D172 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of editor focus, selection,
  font scaling, long-line wrapping, and QScintilla border metrics before
  closing runtime gates.

## Artifact information

The D172 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `BEB4E9626AE27F7FB3DBDDFE3985D4F3D559F6760B073D3C8ED1887C3A292647`.
- Size: `38551851` bytes.
- Source revision: `tree-sha256:840ab21f827dbf7574dfd7b3bee234c63a897436e645699d123659e7a8be300e`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: editor-stage visual depth is delivered; native
rendering, accessibility, and external release evidence remain open.
