# D119 / ARCH-93 — parent review

## Scope

Reviewed the current changes in:

- `src/quillforge/presentation/workspace_file_activation_coordinator.py`
- `src/quillforge/presentation/main_window.py`
- `src/quillforge/presentation/workspace_panel.py` (signal boundary)
- `src/quillforge/presentation/workspace_surface.py` (callback boundary)

## Findings

- PASS: the new coordinator is Qt-free, generic, frozen/slotted-port based,
  and owns no duplicate workspace/document state.
- PASS: the six activation branches preserve the old order: type, startup,
  busy, containment, existing-tab focus/notice, then `_start_open`.
- PASS: `WorkspacePanel` still separates file first-click from directory
  navigation and preserves double-click/keyboard activation semantics.
- PASS: `WorkspaceSurface.file_requested` binds directly to the new seam;
  `MainWindow` retains WorkspaceService, tab surface, notification, and
  asynchronous DocumentOpenCoordinator policy.

## Review result

`PASS` within the bounded source scope. Native tree event ordering,
filesystem behavior, worker interleaving, startup, and close behavior remain
unproven under the no-launch boundary.

## Public-source applicability

Python 3.12/PyQt6 desktop presentation only. Embedded C/C++, MCU, vendor,
firmware, and manufacturer requirements are not applicable. Public CloudWeGo
material remains an engineering reference; no private ByteDance standard or
certification/compliance claim is made.

## Simplification

`PASS`: removing the mixed `_open_workspace_file` method reduces shell policy
coupling while retaining one canonical `_start_open` path and existing
WorkspaceService/DocumentTabSurface owners.
