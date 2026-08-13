# D139 / ARCH-119 parent review

- Reviewer: parent agent
- Result: PASS
- Scope: `workspace_picker_admission_coordinator.py`, MainWindow workspace
  picker wiring, `FileDialogSurface`, and
  `WorkspaceNavigationAdmissionCoordinator`

## Findings

- `WorkspacePickerAdmissionPorts` is frozen/slotted and Qt-free; the native
  `FileDialogSurface` is passed as a callback and is not imported by the
  coordinator.
- Gate precedence is explicit: workspace availability, startup restore, busy,
  directory selection, cancellation, then workspace-open handoff.
- Unavailable workspace and startup restore retain the exact message/level
  contracts; busy and cancellation have no picker/open side effects.
- A selected `Path` reaches `_start_workspace_open`, which remains the
  `WorkspaceNavigationAdmissionCoordinator` boundary.
- `choose_document`/`getOpenFileName` and `choose_workspace`/
  `getExistingDirectory` remain separate in `FileDialogSurface`, preserving the
  reported file-versus-folder boundary.
- MainWindow now exposes one named delegation point while retaining all native
  dialog, navigation, service, result, and close policy ownership.

## Simplification assessment

`PASS`: one focused coordinator and one named ports contract replace only the
composition-root picker sequence. No generic picker hierarchy, duplicated
workspace-open pipeline, Qt import, or business-policy relocation was
introduced. No further safe behavior-preserving simplification was identified.

## Limits

This is static/source/package evidence only. No QApplication/native dialog,
filesystem access, queued worker timing, clean-machine, cross-machine,
installer, signing, updater, or runtime visual evidence was authorized.
