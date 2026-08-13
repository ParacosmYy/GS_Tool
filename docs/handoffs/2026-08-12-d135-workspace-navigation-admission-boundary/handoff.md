# Handoff: 2026-08-12-d135-workspace-navigation-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d135-workspace-navigation-admission-boundary |
| Delivery / slice | D135 / ARCH-113 workspace-navigation admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T03:00:00+08:00 |

## User outcome

Workspace open and directory navigation now have one explicit typed admission
boundary. Availability, busy/startup-restore gating, loading projection,
operation identity, generation binding, and asynchronous dispatch remain
ordered while file/folder semantics and completion policy stay unchanged.

## Scope and boundaries

### In scope

- `WorkspaceNavigationAdmissionCoordinator` and
  `WorkspaceNavigationAdmissionPorts`.
- `MainWindow` composition wiring and workspace open/directory delegation.
- Static behavior/contract/Qt-free probes, package, and traceability records.

### Out of scope

- No `WorkspaceService`, `WorkspaceSurface`,
  `WorkspaceNavigationCoordinator` result classification,
  `WorkspaceNavigationProjectionCoordinator`, containment, cancellation,
  session persistence, notification, close, or application-policy rewrite.
- No QApplication launch, native workspace rendering, filesystem durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Huygens the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Beauvoir the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_navigation_admission_coordinator.py`
  — new Qt-free admission contract and sequencing.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  workspace open/directory delegation only.
- `docs/adr/0175-workspace-navigation-admission-boundary.md`
- `docs/agent-team/reviews/D135-workspace-navigation-admission-parent-review.md`
- `docs/agent-team/reviews/D135-workspace-navigation-admission-independent-review.md`

## Decisions and constraints

- Admission and completion remain separate contracts; result classification
  stays in `WorkspaceNavigationCoordinator`.
- The workspace service is captured after admission and before dispatch, so
  each operation keeps the prior service-binding behavior.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D135-WORKSPACE-ADMISSION-BEHAVIOR-PROBE=PASS`
- `D135-WORKSPACE-ADMISSION-CONTRACT-PROBE=PASS`
- `D135-QT-FREE-NAVIGATION-PROBE=PASS`
- `D135-COMPILEALL=PASS`
- `D135-RUFF=PASS`
- `D135-FORMAT=PASS`
- `D135-CHECK=PASS`
- `D135-VERIFY-HANDOFF=PASS`
- `D135-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect and independent review conclusions — child windows timed out;
  recorded as `NO_CONCLUSION`, not PASS.
- QApplication startup, native workspace interaction, worker callback timing,
  filesystem behavior, keyboard/accessibility traversal, DPI/font fallback,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner checks — prohibited or outside authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static admission evidence cannot prove native WorkspaceSurface event order or
  queued worker timing on every Windows environment.
- Huygens architecture and Beauvoir independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S179`, `D135-AC01`.
- Evidence: ADR-0175, workspace-admission behavior/contract/Qt-free probes,
  parent and independent review records, simplification assessment, static
  checks, package identity, handoff/index/register checks, expected release
  NO-GO, and explicit runtime limits.

## Next owner and next action

- Owner: Architect.
- Action: continue the next bounded MainWindow/application or distinct visual
  gap slice and complete authorized runtime/release gates when authority and
  environment permit.

## Artifact information

The portable candidate was rebuilt without launching QuillForge after the
source change:

- Artifact: `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `D2DFD10F0BFF9C4D0DF2C791BEB4B6ED3288B3F55F0FA6EE596D96BF564E011B`
- Size: `38522779` bytes
- Source revision: `tree-sha256:654513d75a424f50f7ec4cffb60c070337e26634ace52ebae5e8cfae214d9788`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace navigation admission is centralized at a
typed presentation boundary while native/runtime/release evidence remains
open.
