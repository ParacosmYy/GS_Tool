# ADR-0110: Workspace-navigation coordinator boundary

- **Status:** accepted-with-limits; D85 / ARCH-60 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained the completion side of workspace open and directory
navigation: generic operation completion, workspace tracker classification,
loading/error projection, result-shape validation, session-restore release,
and valid-result projection. It also owns the workspace service, TaskRunner,
workspace activation, search-root invalidation, containment, file opening,
notifications, and close policy.

## Decision

Extract the common workspace-navigation completion lifecycle into the Qt-free
`WorkspaceNavigationCoordinator`. It receives the existing
`WorkspaceOperationTracker`, the generic operation-completion seam, a minimal
workspace-navigation surface Protocol, valid-open and valid-directory policy
callbacks, session-restore continuation, and the notification sink.

The coordinator handles stale/invalidated/current classification, loading
release, invalid open/directory result projection, and worker failure
projection. MainWindow keeps valid workspace activation, search-root
invalidation, directory-root selection, session-save scheduling, and all
workspace/application policy.

## Invariants

1. `presentation/workspace_navigation_coordinator.py` imports no PyQt6 or
   widget implementation and exposes only a narrow `Path`-typed surface
   Protocol.
2. A stale callback produces no surface, notification, or session-restore
   side effect.
3. An invalidated open callback releases loading and finishes session restore,
   but never activates a stale workspace result.
4. A current open result must be a `WorkspaceState` and a live surface must
   exist; invalid results preserve the existing error strings and notification.
5. A current directory result must be a `WorkspaceDirectory`; invalid results
   preserve the existing error strings and notification.
6. Current worker failures preserve surface error projection, error-level
   notification, and session-restore continuation; invalidated failures do not
   notify as current failures.
7. MainWindow retains WorkspaceService, TaskRunner, workspace activation,
   search invalidation/root projection, file opening, containment, session
   save, and close policy.

## Alternatives considered

- **Leave both completion callbacks in MainWindow:** rejected; shared
  lifecycle and validation are a focused framework-neutral boundary.
- **Move workspace activation or root policy into the coordinator:** rejected;
  those are application/workspace policy and would increase coupling.
- **Inject `WorkspaceSurface` directly:** rejected; a Protocol keeps Qt object
  ownership in the presentation composition root.
- **Move generic busy/status policy into the coordinator:** rejected; the
  existing `OperationTracker` seam remains the single generic operation owner.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Carver the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`. A follow-up correction architecture window for the local
  `surface`/`Path` fix was Franklin the 3rd / Luna max and also returned
  `NO_CONCLUSION`; no child architecture PASS is claimed.
- Independent review: Bernoulli the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for callback argument order, stale/invalidated
  lifecycle, loading/error ordering, exact invalid-result behavior, failure
  notification policy, and retention of workspace/application policy.
- Simplification assessment: two valid-result callbacks and one failure path
  now share one coordinator without moving success policy or generic operation
  state. No further behavior-preserving reduction was identified.

## Verification target and limits

- `D85-WORKSPACE-NAVIGATION-QT-FREE-BOUNDARY-PROBE=PASS` covers the Qt-free
  import boundary, callback wiring, tracker ownership, old-callback removal,
  and retained MainWindow seams.
- Targeted compileall, Ruff, format, package identity, traceability, handoff,
  repository checks, no-process, and expected release no-go evidence are
  recorded in the D85 handoff.
- Native dialog rendering, callback timing, cancellation interleaving,
  accessibility, DPI, font metrics, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release
  owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
