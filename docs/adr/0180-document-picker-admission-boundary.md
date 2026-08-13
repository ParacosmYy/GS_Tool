# ADR-0180: document-picker admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D138 / ARCH-118

## Context

`MainWindow._open_document` still combined busy and startup-restore admission,
the native file picker, cancellation handling, and the handoff to
`_start_open`. The existing `FileDialogSurface` owns native dialog
presentation, while `DocumentOpenAdmissionCoordinator` owns the asynchronous
open operation boundary. The picker seam was the remaining composition-root
piece and also made the file-open path harder to inspect for the reported
file/folder confusion.

## Decision

Add the Qt-free `DocumentPickerAdmissionCoordinator` and frozen/slotted
`DocumentPickerAdmissionPorts` contract. Preserve this exact sequence:

1. silently reject while another operation is busy;
2. reject startup restore with the existing
   `Restoring the previous session...` warning;
3. ask `FileDialogSurface` to choose one file;
4. treat cancellation as a no-op; and
5. pass the selected `Path` to the existing `_start_open`/
   `DocumentOpenAdmissionCoordinator` boundary.

`FileDialogSurface`, `DocumentService`, document-open result classification,
workspace folder selection, notifications, and close policy remain in their
existing owners. The directory picker remains the separate workspace flow.

## Alternatives rejected

- Moving `QFileDialog` into a Qt-free coordinator would violate the
  presentation boundary.
- Merging file and workspace pickers would blur the file/directory contract
  that distinguishes document opening from workspace navigation.
- Rewriting `DocumentOpenAdmissionCoordinator` would duplicate an existing
  asynchronous operation boundary for a picker-only concern.

## Review and evidence

Hegel the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded review window. The independent review is
recorded separately. Parent review is `PASS`; simplification assessment is
`PASS` because one small typed admission boundary removes only the picker
sequence from `MainWindow` and leaves native dialog/open operation policy in
place.

Authorized non-destructive evidence: `D138-DOCUMENT-PICKER-BEHAVIOR-
PROBE=PASS`, `D138-DOCUMENT-PICKER-CONTRACT-PROBE=PASS`,
`D138-QT-FREE-PICKER-PROBE=PASS`, `D138-MAINWINDOW-WIRING-PROBE=PASS`,
`D138-COMPILEALL=PASS`, `D138-RUFF=PASS`, `D138-FORMAT=PASS`, project checks,
package identity, no-launch, traceability, and expected release NO-GO. Native
QFileDialog behavior, file-system access, worker timing, and runtime visual
evidence remain unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
