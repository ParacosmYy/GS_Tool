# Handoff: 2026-08-10-ui-52-workspace-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-10-ui-52-workspace-hierarchy` |
| Delivery / slice | `UI-52 workspace resource-manager hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T23:59:00+08:00` |

## User outcome

The workspace explorer now gives the user a clear next action before a folder
is selected, a visible loading state during the initial open, and a deliberate
empty-directory state instead of a blank native tree. The current path and
workspace file list have semantic accessibility names, navigation copy is less
shouty, and the existing file/folder icons and interaction guidance remain.

## Scope and boundaries

### In scope

- Localized workspace tree/path/empty-state semantic identities.
- Mutually exclusive unselected/loading/empty-directory/tree projection.
- Centralized empty-state QSS and existing token-based path/tree hierarchy.
- Calmer workspace copy and existing navigation-control/icon presentation.
- Source, i18n, contrast, static, package, handoff, and expected release
  evidence.

### Out of scope

- No `WorkspaceSurface` callbacks, WorkspaceService, directory paging,
  containment, async operation, session restore, persistence, or close policy
  changed.
- No file/directory activation behavior changed: single-click files,
  double-click folders, and Enter/Return retain the same semantic route.
- No custom tree model/delegate, custom painting, widget-local stylesheet,
  dependency, worker, or test-only asset added.
- No Qt launch, screenshot, native rendering, screen-reader, DPI/font,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Darwin the 3rd / Luna max | Read-only UI-52 boundary consultation; `NO_CONCLUSION` after bounded windows |
| Independent review | Pasteur the 3rd / Luna max | Read-only workspace source review; `NO_CONCLUSION` after bounded windows |
| Parent | Architect | Sole writer, integration, source review, simplification, and verification |

No child PASS is claimed.

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — semantic empty state,
  path/tree accessibility names, visibility projection, and native tree hints.
- `src/quillforge/presentation/i18n.py` — localized workspace hierarchy and
  interaction copy.
- `src/quillforge/presentation/theme.py` — centralized workspace empty-state
  and path hierarchy QSS.
- `docs/adr/0129-workspace-resource-manager-hierarchy.md` — decision,
  invariants, public-source applicability, review, and limits.
- UI-52 parent/independent review records, handoff/index, acceptance/delivery,
  roadmap/spec/task/release records.

## Decisions and constraints

- `WorkspacePanel` remains the presentation owner; `_sync_content_state()` is
  private and only coordinates mutually exclusive widget visibility/text.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, static, packaging, and release
  handoff checks are the permitted validation boundary.
- This is Python/PyQt6 UI code. Embedded C/C++ assurance and vendor
  manufacturer requirements are `N/A` for this slice.
- Qt 6.11.1 `QTreeView`/`QTreeWidget` documentation was used as a public
  first-party engineering reference for the existing native view API, not as a
  manufacturer requirement. Public CloudWeGo material remains an engineering
  reference only; no private ByteDance standard, certification, or compliance
  claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `UI-52-WORKSPACE-SOURCE-PROBE=PASS` | `PASS` | New state/semantic selectors plus unchanged signal routes detected. |
| `UI-52-WORKSPACE-CONTRAST-PROBE=PASS` | `PASS` | Three themes × four accents; minimum checked text/surface ratio 4.53:1. |
| `UI-52-I18N-PROBE=PASS` | `PASS` | New workspace keys resolve in English and Simplified Chinese. |
| `python -m compileall -q src/quillforge` | `PASS` | Static compilation only; no QApplication launch. |
| `uv run ruff check src/quillforge` | `PASS` | All checks passed. |
| `uv run ruff format --check src/quillforge` | `PASS` | 119 files already formatted. |
| `scripts\package.ps1` | `PASS` | Portable candidate rebuilt; root/dist identities match. |
| `UI-52-PACKAGE-IDENTITY-PROBE=PASS` | `PASS` | SHA `7C302083D4650C5F847BAEA9677E29B38DEB8A9CF72FB0865E00A09E156E333C`; 38,491,114 bytes; source `tree-sha256:9f5a5faf37f17ebfcc91b18d3860e72c584473a4aaa622450e3bc1420a339f15`. |
| `PyQt6=6.11.0; Qt=6.11.0` | `PASS` | Version probe imported QtCore only; no QApplication. |
| `UI-52-PACKAGE-NO-LAUNCH-PROBE=PASS` | `PASS` | No QuillForge process was running after packaging. |
| `UI-52-RELEASE-EXPECTED-NO-GO=PASS` | `PASS` | Release dossier decision is `no-go`; three expected mechanical report-binding failures remain. |
| `UI-52-RELEASE-DOSSIER-PROBE=PASS` | `PASS` | Dossier artifact identity matches the manifest; 10 open release gates remain. |

## Unrun checks and reason

- Native Qt layout/QSS specificity, screen-reader traversal, keyboard focus
  painting, fonts, DPI, startup, clean-machine, cross-machine, hardware,
  signing, installer, updater, legal, support, and release-owner checks —
  prohibited or outside current authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project R&D policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static QSS/source checks do not prove native selector specificity, layout
  metrics, font fallback, screen-reader wording, or runtime visual preference.
- Darwin and Pasteur review windows returned `NO_CONCLUSION`; no child PASS is
  claimed. Parent source review and simplification assessment are recorded.
- The portable candidate remains unsigned and release remains `NO-GO`; report
  binding failures and external release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `UI-52-AC01`, `S133`.
- Evidence: ADR-0129, UI-52 source/i18n/contrast probes, parent/independent
  review records, compile/lint/format checks, package identity, handoff/index/
  register checks, expected release NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: synchronize the UI-52 traceability/repository/release checks and
  continue the next bounded MainWindow/application or user-visible slice.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size: `7C302083D4650C5F847BAEA9677E29B38DEB8A9CF72FB0865E00A09E156E333C` /
  `38,491,114` bytes.
- Source revision: `tree-sha256:9f5a5faf37f17ebfcc91b18d3860e72c584473a4aaa622450e3bc1420a339f15`.
- Packaging note: portable one-file candidate rebuilt; signing and installer
  remain open.

## Disposition

`accepted-with-limits`: workspace empty/loading/path/tree hierarchy and copy are
improved through the existing presentation boundary, while native runtime and
enterprise release gates remain open.
