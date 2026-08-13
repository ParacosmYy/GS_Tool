# ADR-0142: Workspace-navigation dispatch callback boundary

- **Status:** accepted-with-limits; D114 / ARCH-86 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

MainWindow still assembled the TaskRunner callback closures for opening a
workspace and loading a directory even though
WorkspaceNavigationCoordinator already owned operation completion, generation
classification, loading/error projection, result validation, and
session-restore continuation. The duplicate callback binding made the
composition root responsible for both dispatch wiring and callback policy.

## Decision

Add typed operation, success, failure, and dispatcher contracts to the
existing Qt-free WorkspaceNavigationCoordinator. Add keyword-only
submit_open(...) and submit_directory(...) methods that bind the existing
operation ID and generation to complete_open(...), complete_directory(...),
and fail(...). Route the two MainWindow worker submissions through those
methods.

The coordinator remains a callback-boundary owner only. MainWindow retains
admission and busy state, operation IDs, generation allocation,
WorkspaceService, TaskRunner, surface ownership, root containment, session
barriers, persistence, notifications, and close policy.

## Invariants

1. The coordinator remains free of PyQt6, widgets, TaskRunner, filesystem
   policy, and WorkspaceService dependencies.
2. Open and directory dispatcher callbacks receive the same operation ID and
   the generation captured at admission; stale callbacks produce no surface,
   notification, or restore side effect.
3. Current open results remain WorkspaceState-validated, current directory
   results remain WorkspaceDirectory-validated, and existing invalid-result
   messages are preserved.
4. Loading release, current failure notification, invalidated completion, and
   session-restore continuation remain in the existing coordinator paths.
5. A synchronous dispatcher exception propagates to the caller; the
   coordinator does not introduce retry, queue, or error-swallowing policy.

## Alternatives considered

- **Keep the two callback closures in MainWindow:** rejected; the existing
  coordinator is already the lifecycle owner and can bind both paths without
  moving workspace policy.
- **Create a generic runner adapter or service locator:** rejected; it would
  add an abstraction broader than this duplicate binding and obscure ownership.
- **Move WorkspaceService or TaskRunner into the coordinator:** rejected;
  concrete application policy and worker lifetime belong to MainWindow.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance workflow and simplifier are therefore N/A for source scope; their
applicability gate was recorded and no embedded source was changed. Public
CloudWeGo material remains an engineering reference only. No private
ByteDance standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or
compliance claim is made.

## Review and simplification

- Architect: Boyle the 4th / Luna max; bounded read-only consultation timed
  out and was closed. Status is NO_CONCLUSION; no child architecture PASS is
  claimed.
- Independent review: Pascal the 4th / Luna max; status is recorded in the
  D114 independent-review record after the bounded read-only window. No child
  PASS is claimed unless the record explicitly reports one.
- Parent source review: PASS for typed callback binding, open/directory
  operation-generation pairing, stale/invalidated/current ordering, error and
  restore behavior, dependency direction, and synchronous exception
  propagation.
- Simplification assessment: PASS. Extending the existing coordinator with
  two small typed binding methods is the smallest complete change; no generic
  runner wrapper, second coordinator, new state, or compatibility shim is
  needed.

## Verification target and limits

- Authorized non-destructive evidence includes the D114 production-class
  inline probe, source/dependency probe, compileall, Ruff, format, existing
  presentation-contract audit, package identity, handoff/register/index
  synchronization, repository checks, no-process evidence, and expected
  release NO-GO evidence.
- The inline probe covers valid open/directory results, invalid results,
  current failure, generation-mismatch invalidation, cancelled/stale
  callbacks, and dispatcher exception propagation.
- Not proven: native Qt/TaskRunner timing, actual workspace rendering,
  filesystem durability, QApplication startup, accessibility/DPI/font
  behavior, clean-machine/cross-machine behavior, signing, installer,
  updater, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
