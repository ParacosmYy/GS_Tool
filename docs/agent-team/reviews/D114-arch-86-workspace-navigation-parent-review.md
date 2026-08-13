# D114 / ARCH-86 parent review: workspace-navigation dispatch boundary

## Scope

Reviewed D114 changes in
src/quillforge/presentation/workspace_navigation_coordinator.py and
src/quillforge/presentation/main_window.py against the D114 specification.

## Findings

| Axis | Result | Evidence |
|---|---|---|
| Contract shape | PASS | Typed operation, success, failure, and dispatcher aliases are explicit; submit_open(...) and submit_directory(...) bind existing completion/failure methods. |
| Lifecycle/order | PASS | MainWindow still admits the operation and generation before dispatch; the coordinator preserves tracker completion before loading release and result projection. |
| Open/directory behavior | PASS | Valid WorkspaceState and WorkspaceDirectory results use the existing projection seams; invalid results preserve existing error text and notification policy. |
| Stale/invalidation/error | PASS | Stale/cancelled callbacks are suppressed; generation-mismatch paths release loading and preserve restore/error ordering; current failures notify at error level. |
| Dependency direction | PASS | The coordinator remains Qt-free and does not import TaskRunner, WorkspaceService, filesystem traversal, or widget implementations; MainWindow retains concrete policy. |
| Exception behavior | PASS | Dispatcher exceptions propagate synchronously and are not swallowed or converted into a new retry policy. |

## Simplification assessment

PASS. The existing coordinator is the only appropriate lifecycle owner. Two
small typed binding methods remove duplicate MainWindow callback closures
without adding a runner wrapper, generic abstraction, state, or compatibility
layer.

## Review limits

Native queued timing, QApplication startup, actual workspace navigation,
filesystem durability, accessibility/DPI/font rendering, clean-machine,
cross-machine, and release-owner evidence were not run under the active
no-launch/authorization boundary. The delegated architect consultation
returned NO_CONCLUSION; no child PASS is claimed. Independent-review status
is carried in the separate D114 record.

## Applicability

Python/PyQt6 desktop code only. Embedded C/C++ assurance and manufacturer
requirements are not applicable. Public CloudWeGo material is an engineering
reference only and not a private ByteDance standard or compliance basis.
