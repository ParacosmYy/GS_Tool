# Handoff: 2026-08-11-d174-workspace-dock-visual-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-11-d174-workspace-dock-visual-hierarchy` |
| Delivery / slice | `D174 / UI-86 / ARCH-161 Workspace-dock visual hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | `Current local checkout only` |
| Created | `2026-08-11T17:30:00+08:00` |

## User outcome

The left Workspace dock now has a lighter frame, a calmer title rail, more
balanced title spacing, and native close/float controls that match the modern
document-tab interaction rhythm. Workspace navigation and file behavior are
unchanged.

## Scope and boundaries

### In scope

- `src/quillforge/presentation/theme.py` WorkspaceDock QSS.
- Dock frame/title hierarchy, native close/float sizing, radius, and spacing.
- Static selector and contrast projection across supported themes and accents.

### Out of scope

- No `workspace_surface.py`, workspace tree model, file/folder activation,
  search, locale, signals, docking policy, application, or persistence change.
- No GUI/EXE launch, screenshot, unit test, test asset, or hardware action.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Euler the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Independent review | Jason the 5th / Luna max | `NO_CONCLUSION` after bounded waits; no child PASS claimed |
| Parent | Architect | `PASS`; sole writer, integration, simplification, verification |

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralized WorkspaceDock visual
  hierarchy only.
- D174 ADR, parent/independent review records, and traceability files.

## Decisions and constraints

- `WorkspaceSurface` remains the owner of docking, workspace resources,
  activation, search, locale, and lifecycle semantics; `theme.py` owns only
  visual rules.
- Existing readable foreground derivation remains authoritative for 砂金,
  paper-sand, and all other theme/accent endpoints.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, package, and static evidence
  are the authorized validation boundary.

## Verification commands and results

- `D174-QSS-CONTRACT-PROBE=PASS`.
- `D174-WORKSPACE-CONTRAST-PROBE=PASS` across 3 themes × 4 accents; effective
  minimum was 4.53 for the Paper/Sand violet path-text pair.
- `D174-COMPILEALL=PASS`.
- `D174-RUFF=PASS` and `D174-FORMAT=PASS`.
- `D174-PACKAGE-BUILD=PASS`.
- `D174-PACKAGE-IDENTITY-PROBE=PASS`.
- `D174-CHECK=PASS`, `D174-VERIFY-HANDOFF=PASS`.
- Release verifier remains expected `NO-GO`; stale artifact-bound runtime
  reports and ten open gates are not rewritten.

## Unrun checks and reason

- Native Qt docking/title-button painting/layout, metrics, screen-reader
  output, DPI, accessibility, clean-machine, cross-machine, signing,
  installer, updater, legal, support, release-owner, and runtime startup —
  prohibited or require an unavailable authorized environment.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  the active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6.

## Known risks and limits

- Native Qt styles may produce different dock-title height, subcontrol spacing,
  or button metrics than the static projection; runtime visual review remains
  open.
- Larger user-selected fonts may change title/button fit; native DPI and font
  metrics remain unmeasured.
- The candidate remains unsigned and release remains NO-GO while artifact,
  clean-machine, legal, installer/update, support, and release-owner gates
  are open.

## Acceptance and evidence IDs

- Acceptance: `S227`, `D174-AC01`.
- Evidence: ADR-0223, parent/independent review records, D174 probes,
  `scripts/check.ps1`, release dossier, package identity, handoff/index/register
  checks, and explicit no-go limits.

## Next owner and next action

- Owner: Project Manager / QA / Release Engineering as applicable.
- Action: authorize native Windows visual review of dock-title height,
  close/float metrics, localized widths, screen-reader output, and DPI
  behavior before closing runtime gates.

## Artifact information

The D174 candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`.
- SHA-256: `C15CBE1C3817CE754CFFCAE48C2257AD97F9514C9D5B002B73FFC3ACDF046265`.
- Size: `38550852` bytes.
- Source revision: `tree-sha256:34512f058e1e2c048a24b2f0c54b6f05601fb81a425e61f53410cf70182084b0`.
- Manifest: `dist/QuillForge.release.json`.

## Disposition

`accepted-with-limits`: Workspace-dock visual hierarchy is delivered; native
rendering, accessibility, and external release evidence remain open.
