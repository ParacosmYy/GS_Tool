# ADR-0175: workspace-navigation admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D135 / ARCH-113

## Context

`MainWindow._start_workspace_open` and `_load_workspace_directory` still
combined workspace availability checks, the startup-restore barrier, loading
projection, generic operation allocation, workspace-generation binding, and
asynchronous submission. The existing
`WorkspaceNavigationCoordinator` already owns completion classification, so
the admission sequence was the remaining composition-root seam.

## Decision

Add the Qt-free `WorkspaceNavigationAdmissionCoordinator` and frozen/slotted
`WorkspaceNavigationAdmissionPorts` contract. The coordinator owns only this
request sequence:

1. reject non-`Path` directory signals before touching navigation state;
2. capture the current workspace service and startup-restore state;
3. reject missing workspace/surface, busy work, or ordinary requests during
   startup restore, retaining the exact warning text;
4. set the existing workspace surface to loading;
5. begin the existing generic operation with the former open/directory text;
6. bind a generation through `WorkspaceOperationTracker`; and
7. submit the captured service operation through the existing
   `WorkspaceNavigationCoordinator` and `TaskRunner` dispatcher.

Workspace-open retains the explicit `session_restore` exception. Directory
navigation remains an ordinary request. `WorkspaceService` containment and
filesystem behavior, `WorkspaceNavigationCoordinator` result classification,
`WorkspaceNavigationProjectionCoordinator`, cancellation, session restore,
notifications, and close policy remain in their existing owners.

## Alternatives rejected

- Keeping both sequences in `MainWindow` leaves admission coupled to the
  composition root and duplicates the open/directory gate ordering.
- Moving completion classification or projection into the admission module
  would merge request admission with result policy and session behavior.
- A generic event bus or variadic port would hide the distinct open/directory
  submission contracts and weaken operation/generation observability.

## Review and evidence

Huygens the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Beauvoir the 4th /
Luna max was assigned the independent read-only review and also returned no
conclusion; no child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because one explicit typed admission boundary replaces
two composition-root sequences without duplicating completion policy.

Authorized non-destructive evidence: `D135-WORKSPACE-ADMISSION-BEHAVIOR-
PROBE=PASS`, `D135-WORKSPACE-ADMISSION-CONTRACT-PROBE=PASS`,
`D135-QT-FREE-NAVIGATION-PROBE=PASS`, `D135-COMPILEALL=PASS`,
`D135-RUFF=PASS`, `D135-FORMAT=PASS`, project checks, package identity,
no-launch, traceability, and expected release NO-GO. Native workspace
rendering, QApplication startup, queued worker timing, filesystem behavior,
clean-machine, cross-machine, and release-owner evidence remain unrun under
the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
