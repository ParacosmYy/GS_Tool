# D115 / ARCH-89 parent review: workspace-search dispatch boundary

## Scope

Reviewed D115 changes in
src/quillforge/presentation/workspace_search_coordinator.py and
src/quillforge/presentation/main_window.py against the D115 specification.

## Findings

| Axis | Result | Evidence |
|---|---|---|
| Contract shape | PASS | Typed operation/success/failure/dispatcher aliases and keyword-only submit(...) are explicit. |
| Lifecycle/order | PASS | MainWindow admits operation/generation and busy state before submit; coordinator retains tracker classification before surface/notification projection. |
| Search behavior | PASS | Valid result, zero-match warning, invalid result, worker failure, cancellation, stale, and invalidated paths retain existing semantics. |
| Dependency direction | PASS | Coordinator remains Qt-free and independent of WorkspaceSearchService, TaskRunner, filesystem, and widgets; MainWindow retains service/query/cancellation policy. |
| Exception behavior | PASS | Dispatcher exceptions propagate synchronously and are not converted into a retry or notification policy. |
| Simplification | PASS | One submit binding method removes duplicate callback closures without adding state or a generic runner layer. |

## Simplification assessment

PASS. The existing WorkspaceSearchCoordinator is the lifecycle owner. The
single typed submit(...) boundary is the smallest complete change.

## Review limits

Native queued timing, actual filesystem search, QApplication startup, dialog
rendering, accessibility/DPI/font metrics, clean-machine, cross-machine, and
release-owner evidence were not run under the active no-launch/authorization
boundary. Delegated review statuses are carried by the separate D115 records;
no child PASS is claimed without explicit evidence.

## Applicability

Python/PyQt6 desktop code only. Embedded C/C++ assurance and manufacturer
requirements are not applicable. Public CloudWeGo material is an engineering
reference only and not a private ByteDance standard or compliance basis.
