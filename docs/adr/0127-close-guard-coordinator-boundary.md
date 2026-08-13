# ADR-0127: Close-readiness coordinator boundary

- **Status:** accepted-with-limits; D102 / ARCH-76 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow.closeEvent` carried the complete close-readiness policy: active
operation precedence, cooperative workspace-search cancellation, dirty-tab
protection, recovery/settings/plugin background guards, immediate session-save
admission, queued-completion recheck, and timer cleanup. The Qt event method
therefore mixed deterministic policy with event acceptance and error-dialog
projection.

## Decision

Extract the ordered policy into the Qt-free `CloseGuardCoordinator`. It returns
`CloseGuardDecision` and keeps the existing precedence and side effects:

1. an active foreground operation blocks immediately;
2. an in-flight workspace search is asked to cancel, then close blocks;
3. dirty or editor-modified tabs block;
4. recovery, settings, or plugin background work blocks;
5. an immediate session-save request is made and pending worker/completion work
   is rechecked;
6. only after all gates clear are the recovery and session-save timers stopped
   and close allowed.

`MainWindow` remains the composition root. It supplies concrete state and
callbacks, maps a typed block reason to the existing error-dialog strings, and
accepts or ignores `QCloseEvent`. No message localization, session schema,
TaskRunner, cancellation, timer, or close behavior is changed.

## Invariants

1. `close_guard_coordinator.py` imports no PyQt6, widget, `QCloseEvent`, timer,
   TaskRunner, service, or persistence implementation.
2. The coordinator evaluates each gate at most once per call and preserves the
   old short-circuit order; later side effects never run after an earlier block.
3. Workspace-search cancellation remains cooperative; the coordinator does not
   wait, terminate, or mutate worker state directly.
4. The immediate session-save request occurs only after all earlier close gates
   clear; pending work is checked after that request, exactly as before.
5. Qt event acceptance, localized/error feedback, concrete timers, session
   persistence, operation ownership, and close guards remain in MainWindow.

## Alternatives considered

- **Keep the whole policy in `closeEvent`:** rejected; deterministic close
  precedence remains coupled to Qt event plumbing and is difficult to review as
  one contract.
- **Move `QCloseEvent` or QMessageBox behavior into the coordinator:** rejected;
  it would reverse the presentation boundary and leak Qt into policy code.
- **Create a generic application shutdown service:** rejected; this slice is
  only the existing desktop close decision and adds no process/session use case.
- **Return localized strings from the coordinator:** rejected; MainWindow keeps
  locale and message-surface ownership, preventing a second translation path.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance gate is recorded as N/A for this change.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: James the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No child architecture
  PASS is claimed.
- Independent review: Nietzsche the 3rd / Luna max; two bounded read-only
  waits returned `NO_CONCLUSION`, then the agent was closed. No independent
  PASS is claimed.
- Parent source review: PASS for exact close precedence, cancellation and
  session-save timing, pending-work recheck, timer cleanup, MainWindow Qt
  ownership, and absence of behavior changes.
- Simplification assessment: PASS. The boundary contains only one close
  decision and its unavoidable ordered callbacks; no context bag, shutdown
  framework, duplicate message catalog, or new state model was added. No
  further safe reduction was identified.

## Verification target and limits

- `D102-CLOSE-GUARD-SOURCE-PROBE=PASS` covers the Qt-free module and direct
  MainWindow integration.
- `D102-CLOSE-GUARD-ORDER-PROBE=PASS` covers all five blocking paths and the
  allow path with exact callback order.
- Compileall, Ruff, format, package identity, handoff, repository,
  no-process, traceability, and expected release NO-GO evidence are recorded in
  the D102 handoff.
- Native Qt close-event delivery, QMessageBox rendering, worker interleavings,
  accessibility, DPI, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
