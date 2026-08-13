# Handoff: 2026-08-12-d140-workspace-search-surface-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d140-workspace-search-surface-admission-boundary |
| Delivery / slice | D140 / ARCH-121 workspace-search surface admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T07:00:00+08:00 |

## User outcome

Workspace search surface admission now has one explicit typed boundary. The
shell keeps the existing restore/error/warning behavior, creates or reuses the
non-modal search surface, invalidates stale search scope when the workspace
root changes, and presents the surface without moving search execution policy.

## Scope and boundaries

### In scope

- `WorkspaceSearchSurfaceAdmissionCoordinator`, its ports, and protocol.
- MainWindow composition wiring and `_show_workspace_search` delegation.
- Static behavior/contract/Qt-free/wiring probes, package, and records.

### Out of scope

- No `WorkspaceSearchSurface`, `WorkspaceSearchDialog`, query validation,
  operation tracker, cancellation, worker dispatch, result classification,
  containment, locale projection, notifications, or close-policy rewrite.
- No QApplication/native search dialog launch, file-system durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Ptolemy the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Singer the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_search_surface_admission_coordinator.py` —
  new Qt-free surface admission contract and sequence.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_show_workspace_search` delegation only.
- `docs/adr/0183-workspace-search-surface-admission-boundary.md`
- `docs/agent-team/reviews/D140-workspace-search-surface-admission-parent-review.md`
- `docs/agent-team/reviews/D140-workspace-search-surface-admission-independent-review.md`

## Decisions and constraints

- Qt surface construction remains in MainWindow's presentation composition
  callback; the coordinator depends only on a narrow surface protocol.
- Search execution remains in `WorkspaceSearchCoordinator`,
  `WorkspaceSearchOperationTracker`, and the existing TaskRunner path.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D140-WORKSPACE-SEARCH-SURFACE-BEHAVIOR-PROBE=PASS`
- `D140-WORKSPACE-SEARCH-SURFACE-CONTRACT-PROBE=PASS`
- `D140-QT-FREE-WORKSPACE-SEARCH-PROBE=PASS`
- `D140-MAINWINDOW-WORKSPACE-SEARCH-WIRING-PROBE=PASS`
- `D140-COMPILEALL=PASS`
- `D140-RUFF=PASS`
- `D140-FORMAT=PASS`
- `D140-CHECK=PASS`
- `D140-VERIFY-HANDOFF=PASS`
- `D140-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect conclusion — child window timed out; recorded as `NO_CONCLUSION`.
- Independent review conclusion — child window timed out; recorded as
  `NO_CONCLUSION`, not PASS.
- QApplication/native search dialog, file-system behavior, queued worker
  timing, runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes cannot prove native search-dialog behavior or worker
  timing on every Windows environment.
- Ptolemy architecture and Singer independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S187`, `D140-AC01`.
- Evidence: ADR-0183, surface behavior/contract/Qt-free/wiring probes, parent
  and independent review records, simplification assessment, static checks,
  package identity, handoff/index/register checks, expected release NO-GO, and
  explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or visual-quality
  slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The candidate was rebuilt after the source change without launching
QuillForge:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `204878A997289CEB8D3773DA05C06164E532CE767318E3623ED2AB681F884E9C`
- Size: `38529941` bytes
- Source revision: `tree-sha256:1378abd2f1d72b8c4b6c7ac1fe5e42e8a05560f664084402b5af7cfc38658a10`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace-search surface admission is centralized and
typed; native dialog/runtime/release evidence remains open.
