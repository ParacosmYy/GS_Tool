# ADR-0124: Workspace-navigation projection coordinator boundary

- **Status:** accepted-with-limits; D99 / ARCH-73 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D85 moved workspace navigation callback classification into
`WorkspaceNavigationCoordinator`, including operation identity, stale and
invalidated completion, loading release, invalid results, failures, and session
restore release. The valid `WorkspaceState` and `WorkspaceDirectory` branches
still kept workspace activation, search-root projection, directory projection,
success feedback, session persistence, and restore continuation in
`MainWindow`.

## Decision

Extract those valid branches into the Qt-free
`WorkspaceNavigationProjectionCoordinator`. For a validated workspace-open
result it preserves this order:

1. invalidate the previous workspace-search generation;
2. activate the application workspace;
3. update the search surface root;
4. preserve the existing missing-surface early return;
5. project the returned directory;
6. notify success;
7. request session persistence; and
8. release the pending session-restore workspace step.

For a validated directory result it obtains the current workspace root and
projects only when both the root and workspace surface exist. Concrete service,
surface, search, notification, persistence, and session callbacks remain
explicit constructor dependencies.

`WorkspaceNavigationCoordinator` remains responsible for operation completion,
stale/invalidation/loading classification, invalid result and failure
projection, tracker ownership, and failure-time session restore release.
`MainWindow` remains the composition root and retains admission guards,
generation/TaskRunner wiring, WorkspaceService calls, concrete surfaces,
containment, and close policy.

## Invariants

1. `workspace_navigation_projection_coordinator.py` imports no PyQt6, widget,
   workspace surface, filesystem provider, EventBus, or persistence
   implementation.
2. D85 invokes the projection coordinator only after a current, typed result;
   stale, invalidated, invalid, and failed callbacks never reach it.
3. A missing workspace surface keeps the former valid-open early return; it
   does not emit success, schedule session persistence, or finish restore.
4. A directory result with no current workspace root or surface remains a
   no-op, matching the former MainWindow method.
5. No workspace containment, directory paging, search cancellation, session
   schema, notification contract, or close guard changes.

## Alternatives considered

- **Keep both valid branches in MainWindow:** rejected; it leaves valid
  workspace policy ordering coupled to the Qt composition root after D85 has
  already isolated completion classification.
- **Move valid projection into WorkspaceNavigationCoordinator:** rejected; it
  would merge callback classification with concrete result policy and weaken
  the existing D85 boundary.
- **Create one universal navigation/document operation coordinator:** rejected;
  it would hide distinct workspace/open/save contracts behind broad indirection.
- **Move WorkspaceService or surface objects into the new module:** rejected;
  explicit callbacks preserve application ownership and dependency direction.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance gate is recorded as N/A for this change.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Hypatia the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No child architecture
  PASS is claimed.
- Independent review: Aristotle the 3rd / Luna max; two bounded read-only
  waits returned `NO_CONCLUSION`, then the agent was closed. No independent
  PASS is claimed.
- Parent source review: PASS for open/directory order, missing-surface/root
  no-op behavior, D85 classification ownership, and Qt-free dependencies.
- Simplification assessment: PASS. One focused coordinator expresses two
  related valid-result branches without a universal operation framework or
  shared mutable context. No further safe behavior-preserving reduction was
  identified.

## Verification target and limits

- `D99-WORKSPACE-PROJECTION-SOURCE-PROBE=PASS` covers the Qt-free boundary,
  direct D85 wiring, and removal of the old MainWindow projection methods.
- `D99-WORKSPACE-PROJECTION-ORDER-PROBE=PASS` covers open ordering, directory
  root pass-through, and missing-surface early return.
- Compileall, Ruff, format, package identity, handoff, repository, no-process,
  traceability, and expected release NO-GO evidence are recorded in the D99
  handoff.
- Native workspace/search rendering, callback timing, accessibility, DPI,
  fonts, runtime startup, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
