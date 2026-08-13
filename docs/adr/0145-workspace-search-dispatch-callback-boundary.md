# ADR-0145: Workspace-search dispatch callback boundary

- **Status:** accepted-with-limits; D115 / ARCH-89 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

WorkspaceSearchCoordinator already owned search callback classification,
generation invalidation, cooperative-cancellation completion, result
validation, surface projection, and summary severity. MainWindow still
assembled the TaskRunner success/failure closures and captured generation
there, leaving the composition root responsible for duplicate callback policy.

## Decision

Add typed WorkspaceSearchOperation, WorkspaceSearchSuccess,
WorkspaceSearchFailure, and WorkspaceSearchDispatcher contracts to the
existing Qt-free coordinator. Add keyword-only submit(...) that binds the
admitted generation to the existing complete(...) and fail(...) paths.
MainWindow routes the search operation through submit(...) while retaining
query construction, cancellation token ownership, operation admission,
WorkspaceSearchService, TaskRunner, surface, containment, locale,
notifications, and close policy.

## Invariants

1. The coordinator remains free of PyQt6, TaskRunner, WorkspaceSearchService,
   filesystem, and widget dependencies.
2. The dispatcher receives the admitted operation ID and callbacks retain the
   generation captured before submission; stale and invalidated callbacks keep
   their existing suppression/cancellation behavior.
3. Valid/invalid results, zero-match warning severity, worker failure
   projection, cancellation, summary, and notification order are unchanged.
4. A synchronous dispatcher exception propagates to MainWindow; no retry,
   swallowing, or new queue policy is introduced.
5. MainWindow retains all search query, containment, surface, locale, close,
   and TaskRunner policy.

## Alternatives considered

- **Keep closures in MainWindow:** rejected; it duplicates the lifecycle
  binding already adjacent to the coordinator's classification policy.
- **Move query or cancellation ownership into the coordinator:** rejected;
  those are application/service policy and would increase coupling.
- **Create a generic runner wrapper:** rejected; one search boundary does not
  justify a second abstraction or service locator.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance workflow and simplifier are N/A for source scope; no embedded source
was changed. Public CloudWeGo material remains an engineering reference only.
No private ByteDance standard, certification, MISRA, ISO 26262, ASIL, ASPICE,
or compliance claim is made.

## Review and simplification

- Architect: Harvey the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Hilbert the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent source review: PASS for typed binding, generation/operation pairing,
  stale/invalidated/cancel ordering, zero-match severity preservation,
  dependency direction, and synchronous exception propagation.
- Simplification assessment: PASS. Extending the existing coordinator with
  one submit(...) method is the smallest complete change; no new state,
  generic runner adapter, or parallel coordinator is needed.

## Verification target and limits

- Authorized evidence includes the D115 production-class inline probe, source/
  dependency probe, compileall, Ruff, format, presentation-contract audit,
  package identity, handoff/register/index synchronization, repository checks,
  no-process evidence, and expected release NO-GO evidence.
- Not proven: native TaskRunner timing, actual filesystem search,
  QApplication startup, dialog rendering, accessibility/DPI/font behavior,
  clean-machine/cross-machine behavior, signing, installer, updater, legal,
  support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
