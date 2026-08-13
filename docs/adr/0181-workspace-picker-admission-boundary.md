# ADR-0181: workspace-picker admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D139 / ARCH-119

## Context

`MainWindow._choose_workspace` combined workspace availability, startup-restore
and busy admission, the native directory picker, cancellation, and handoff to
workspace navigation. The preceding D138 slice made the document file-picker
path explicit; the directory picker must remain a separate flow so a selected
file is never treated as a workspace root and a selected folder is never sent
to document opening.

## Decision

Add the Qt-free `WorkspacePickerAdmissionCoordinator` and frozen/slotted
`WorkspacePickerAdmissionPorts` contract. Preserve this exact sequence:

1. reject an unavailable workspace surface with the existing error message;
2. reject startup restore with the existing
   `Restoring the previous session...` warning;
3. silently reject while another operation is busy;
4. ask `FileDialogSurface` to choose one directory;
5. treat cancellation as a no-op; and
6. pass the selected `Path` to `_start_workspace_open`, which remains the
   existing `WorkspaceNavigationAdmissionCoordinator` boundary.

`FileDialogSurface`, `WorkspaceNavigationAdmissionCoordinator`, workspace
service/result classification, file activation, containment, notifications,
session restore, and close policy remain in their existing owners. The
document file-picker and workspace directory-picker contracts remain distinct.

## Alternatives rejected

- Moving `QFileDialog` into the Qt-free coordinator would violate the
  presentation boundary.
- Reusing the document picker coordinator would erase the file/directory
  distinction and route a folder through document opening.
- Rewriting workspace navigation admission would duplicate asynchronous
  operation and generation policy already owned by its coordinator.

## Review and evidence

Nash the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded review window. Euler the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment is
`PASS` because one small typed boundary removes only the picker sequence from
`MainWindow` without introducing a generic picker abstraction or moving native
UI policy.

Authorized non-destructive evidence: `D139-WORKSPACE-PICKER-BEHAVIOR-
PROBE=PASS`, `D139-WORKSPACE-PICKER-CONTRACT-PROBE=PASS`,
`D139-QT-FREE-WORKSPACE-PICKER-PROBE=PASS`,
`D139-MAINWINDOW-WORKSPACE-PICKER-WIRING-PROBE=PASS`, compile/lint/format,
project checks, package identity, no-launch, traceability, and expected release
NO-GO. Native `QFileDialog`, filesystem behavior, worker timing, and runtime
visual evidence remain unrun under the active authorization boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
no embedded C/C++, MCU, RTOS, or manufacturer requirement applies. No external
vendor rule was used as a conformance claim. Public CloudWeGo material remains
an engineering reference only, not a private ByteDance standard or a
certification/compliance claim.
