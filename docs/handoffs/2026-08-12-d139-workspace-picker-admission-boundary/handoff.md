# Handoff: 2026-08-12-d139-workspace-picker-admission-boundary

| Field | Value |
|---|---|
| ID | 2026-08-12-d139-workspace-picker-admission-boundary |
| Delivery / slice | D139 / ARCH-119 workspace-picker admission boundary |
| Status | `accepted-with-limits` |
| Owner | architect |
| Checkout | Current local checkout only |
| Created | 2026-08-12T06:00:00+08:00 |

## User outcome

Workspace folder selection now has one explicit typed admission boundary. A
selected directory is handed to the existing workspace navigation path, while
document file selection remains a separate file-opening flow.

## Scope and boundaries

### In scope

- `WorkspacePickerAdmissionCoordinator` and
  `WorkspacePickerAdmissionPorts`.
- MainWindow workspace-picker composition wiring and `_choose_workspace`
  delegation.
- Static behavior/contract/Qt-free/wiring probes, package, and records.

### Out of scope

- No `FileDialogSurface`, `WorkspaceNavigationAdmissionCoordinator`,
  `WorkspaceService`, directory result classification, file activation,
  containment, notifications, settings, or close-policy rewrite.
- No QApplication/native QFileDialog launch, file-system durability,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.

## Team roles and ownership

| Role | Owner / agent | Result |
|---|---|---|
| Architect | Nash the 4th / Luna max | `NO_CONCLUSION` after bounded window; no child PASS |
| Independent review | Euler the 4th / Luna max | `NO_CONCLUSION` after bounded window; no independent PASS |
| Parent | Architect | `PASS`; sole writer, integration, review, simplification, and verification |

## Changed files and modules

- `src/quillforge/presentation/workspace_picker_admission_coordinator.py` —
  new Qt-free directory-picker admission contract and sequence.
- `src/quillforge/presentation/main_window.py` — named ports wiring and
  `_choose_workspace` delegation only.
- `docs/adr/0181-workspace-picker-admission-boundary.md`
- `docs/agent-team/reviews/D139-workspace-picker-admission-parent-review.md`
- `docs/agent-team/reviews/D139-workspace-picker-admission-independent-review.md`

## Decisions and constraints

- Native directory selection and workspace navigation remain separate
  presentation/application boundaries.
- File selection remains `getOpenFileName`/`choose_document`; directory
  selection remains `getExistingDirectory`/`choose_workspace`.
- The parent is the sole shared-checkout writer. No Git/worktree operation was
  used; the current local checkout is the only workspace.
- Runtime launch remains unauthorized; source, inline, package, and static
  evidence are the authorized validation boundary.

## Verification commands and results

- `D139-WORKSPACE-PICKER-BEHAVIOR-PROBE=PASS`
- `D139-WORKSPACE-PICKER-CONTRACT-PROBE=PASS`
- `D139-QT-FREE-WORKSPACE-PICKER-PROBE=PASS`
- `D139-MAINWINDOW-WORKSPACE-PICKER-WIRING-PROBE=PASS`
- `D139-COMPILEALL=PASS`
- `D139-RUFF=PASS`
- `D139-FORMAT=PASS`
- `D139-CHECK=PASS`
- `D139-VERIFY-HANDOFF=PASS`
- `D139-PACKAGE-BUILD=PASS`

## Unrun checks and reason

- Architect conclusion — child window timed out; recorded as `NO_CONCLUSION`.
- Independent review conclusion — child window timed out; recorded as
  `NO_CONCLUSION`, not PASS.
- QApplication/native QFileDialog, file-system behavior, queued worker timing,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner checks — prohibited or outside
  authorization.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — prohibited by
  active project policy and not created.
- Embedded target/vendor evidence — not applicable to Python/PyQt6 code.

## Known risks and limits

- Static source probes cannot prove native directory-dialog return values or
  workspace-open worker timing on every Windows environment.
- Nash architecture and Euler independent review windows returned
  `NO_CONCLUSION`; no child PASS is claimed.
- The portable candidate remains unsigned and release remains NO-GO while
  external and report-binding gates are open.

## Acceptance and evidence IDs

- Acceptance: `S185`, `D139-AC01`.
- Evidence: ADR-0181, picker behavior/contract/Qt-free/wiring probes, parent
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
- SHA-256: `867D9F8901B6ED59F909A2A4550B9624AA78BE24900ABC5A59CAF3C4675B43BC`
- Size: `38530288` bytes
- Source revision: `tree-sha256:06ac1f8f7e80540ab3ef09d6c3da53130ec16fce308e549a24e4b2f191509f7c`
- Manifest: `dist/QuillForge.release.json`

## Disposition

`accepted-with-limits`: workspace folder-picker admission is centralized and
typed; native dialog/runtime/release evidence remains open.
