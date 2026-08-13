# ADR-0109: Workspace-search coordinator boundary

- **Status:** accepted-with-limits; D84 / ARCH-59 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained the completion side of Find in Files: operation-tracker
finish classification, invalidated-search feedback, result-shape validation,
surface projection, summary severity, and failure projection. It already kept
query construction, `WorkspaceSearchService`, cooperative cancellation,
TaskRunner submission, root containment, result activation, and close policy.

## Decision

Extract completion projection into the Qt-free `WorkspaceSearchCoordinator`.
It receives the existing `WorkspaceSearchOperationTracker`, a minimal search
surface Protocol, a locale-aware summary callback, and the notification sink.
It preserves stale suppression, invalidated cancellation feedback, typed
`WorkspaceSearchResult` validation, result severity, and failure messages.

Keep MainWindow responsible for query validation, WorkspaceSearchService,
TaskRunner, generation/cancellation inputs, workspace root/containment,
document opening, search-surface construction, and close behavior.

## Invariants

1. `presentation/workspace_search_coordinator.py` imports no PyQt6 or widget
   type.
2. A stale callback produces no surface or notification side effect.
3. An invalidated current callback projects cancellation and never projects a
   stale result or failure.
4. A current result must be a `WorkspaceSearchResult`; invalid results keep
   the existing surface error and error notification.
5. Current valid results preserve surface projection and warning/success
   classification for cancellation, limits, diagnostics, and empty matches.
6. MainWindow retains search service/query/cancel/containment/open-match/close
   policy; the coordinator has no filesystem or editor capability.

## Alternatives considered

- **Leave callbacks in MainWindow:** rejected; result lifecycle projection is a
  focused framework-neutral boundary.
- **Move query construction or cancellation:** rejected; these are request and
  user-action policy owned by MainWindow and the application service.
- **Inject WorkspaceSearchSurface directly:** rejected; a Protocol keeps the
  coordinator independent of Qt object construction and widget ownership.
- **Move summary localization into the coordinator:** rejected; the summary
  callback keeps current locale ownership in MainWindow/presentation i18n.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Faraday the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Bohr the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for tracker lifecycle, callback argument order,
  stale/invalidated behavior, result validation, severity projection, and
  retention of query/cancel/containment/open/close policy.
- Simplification assessment: two MainWindow completion callbacks become one
  coordinator while the summary and surface seams stay explicit. No further
  behavior-preserving reduction was identified.

## Verification target and limits

- `D84-WORKSPACE-SEARCH-BOUNDARY-PROBE=PASS` covers callback wiring, tracker
  ownership, old callback removal, and retained service/cancel/open/close
  policy.
- `D84-WORKSPACE-SEARCH-QT-FREE-PROBE=PASS` confirms bare import without
  PyQt6.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D84 handoff.
- Native dialog rendering, callback timing, cancellation interleaving,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
