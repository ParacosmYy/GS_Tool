# ADR-0235: Workspace file-entry closure

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D186 / UI-97 / ARCH-173

## Decision

Expose a distinct localized `Open file` button in the workspace dock. The
button emits `WorkspacePanel.file_picker_requested`; `WorkspaceSurface`
forwards the semantic callback; `MainWindow` binds it to the existing
`_open_document` method. The existing chain remains authoritative:

`FileDialogSurface.choose_document` →
`DocumentPickerAdmissionCoordinator` →
`DocumentOpenAdmissionCoordinator` → generic worker dispatcher →
`DocumentService.open_document`.

The folder button remains the workspace chooser. Workspace tree file click,
double-click, and keyboard activation remain unchanged. Loading disables the
new file action alongside the existing folder/tree controls. English and
Simplified Chinese labels are added through the existing presentation i18n
catalog.

## Non-goals and evidence

No second file picker, direct service call, new application state, or changed
workspace containment policy is introduced. Evidence includes
`D186-WORKSPACE-FILE-ACTION-PROBE=PASS`,
`D186-SURFACE-CALLBACK-PROBE=PASS`,
`D186-REUSE-OPEN-BOUNDARY-PROBE=PASS`, `D186-I18N-PROBE=PASS en+zh-CN`,
`D186-FILE-DIALOG-CHAIN-PROBE=PASS`,
`D186-FILE-ACTION-QSS-PROBE=PASS`, and
`D186-COMPILE-RUFF-FORMAT=PASS`.

This is Python/PyQt6 presentation/application-boundary work. Embedded C/C++,
MCU, RTOS, and manufacturer requirements are not applicable. Public
engineering guidance is not a private ByteDance standard or compliance claim.
Native dialog behavior, GUI startup, accessibility, DPI, screenshots,
clean-machine, cross-machine, and release evidence remain open.
