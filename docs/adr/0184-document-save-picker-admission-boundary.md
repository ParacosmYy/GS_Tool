# ADR-0184: document-save picker admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D141 / ARCH-122

## Context

`MainWindow._save_document` and `_save_as_document` combined active-tab and
busy guards, ordinary-save target routing, Save As intent, the native save
picker, cancellation, and handoff into the existing document-save boundary.
The existing `DocumentSaveAdmissionCoordinator` already owns validation,
conflict, read-only, persistence, result, and notification policy; only picker
admission remained in the shell composition root.

## Decision

Add the Qt-free generic `DocumentSavePickerAdmissionCoordinator` with frozen/
slotted `DocumentSavePickerAdmissionPorts`. Preserve this exact behavior:

1. silently reject when there is no active tab or another operation is busy;
2. ordinary Save uses the tab's existing path;
3. ordinary Save for an untitled tab invokes the native save picker;
4. Save As always invokes the native save picker with the current path as its
   default; 
5. cancellation is a no-op; and
6. a selected `Path` is handed to `_start_save`, which remains the existing
   `DocumentSaveAdmissionCoordinator` boundary.

`FileDialogSurface`, document validation/conflict/read-only/persistence,
result classification, recovery/session projection, notifications, and close
policy remain in their existing owners. The coordinator has no PyQt6 import.

## Alternatives rejected

- Moving native `QFileDialog` into the Qt-free coordinator would violate the
  presentation boundary.
- Moving save validation or persistence here would duplicate the existing
  `DocumentSaveAdmissionCoordinator` policy.
- Merging Save and Save As into a new dialog policy would erase the explicit
  `force_picker` intent and change command behavior.

## Review and evidence

Fermat the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Dirac the 4th / Luna
max was assigned the independent read-only review and also returned no
conclusion. No child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because one small generic picker boundary isolates only
active-tab/target selection and leaves the existing asynchronous save policy
untouched.

Authorized non-destructive evidence: `D141-DOCUMENT-SAVE-PICKER-BEHAVIOR-
PROBE=PASS`, `D141-DOCUMENT-SAVE-PICKER-CONTRACT-PROBE=PASS`,
`D141-QT-FREE-DOCUMENT-SAVE-PICKER-PROBE=PASS`,
`D141-MAINWINDOW-DOCUMENT-SAVE-PICKER-WIRING-PROBE=PASS`, compile/lint/format,
project checks, package identity, no-launch, traceability, and expected release
NO-GO. Native save dialog, filesystem behavior, worker timing, and runtime
visual evidence remain unrun under the active authorization boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
no embedded C/C++, MCU, RTOS, or manufacturer requirement applies. No external
vendor rule was used as a conformance claim. Public CloudWeGo material remains
an engineering reference only, not a private ByteDance standard or a
certification/compliance claim.
