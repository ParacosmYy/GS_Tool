# ADR-0140: Session-load dispatch callback boundary

- **Status:** accepted-with-limits; D113/ARCH-85 bounded architecture slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`SessionLoadCoordinator` already owns session-load result classification:
`SessionLoadResult` validation, default-baseline projection, invalid-manifest
error reporting, failure projection, and recovery-first continuation. MainWindow
still submits `SessionService.load` directly to `TaskRunner` with the
coordinator callbacks.

## Decision

Extend the existing Qt-free coordinator with typed `SessionLoadOperation`,
`SessionLoadSuccess`, `SessionLoadFailure`, and `SessionLoadDispatcher`
contracts plus a keyword-only `submit(...)` method. It binds the existing
`complete()`/`fail()` callbacks to a structural dispatcher supplied by
MainWindow.

MainWindow now passes `SessionService.load` and `TaskRunner.submit` while
retaining startup admission, operation-ID allocation, session baseline and
restore state, notifications, recovery ordering, persistence, and close
policy. A synchronous dispatcher error propagates without synthesizing
completion or changing restore state.

## Invariants

1. `SessionLoadResult` validation, default baseline, invalid-manifest
   preservation, failure notification, and recovery-first scan continuation
   remain unchanged.
2. Baseline writes continue before recovery scan scheduling.
3. The coordinator remains Qt-free and imports neither TaskRunner,
   SessionService, filesystem, nor widgets.
4. MainWindow keeps startup admission, operation IDs, service/runner,
   persistence, session restore, notifications, and close ownership.

## Alternatives considered

- **Keep direct callback binding in MainWindow:** rejected; the session-load
  lifecycle owner already owns classification and continuation.
- **Create a second startup dispatcher coordinator:** rejected; it would split
  session baseline and recovery-first lifecycle decisions.
- **Move SessionService or startup policy into the coordinator:** rejected; it
  would cross the Qt-free boundary and hide application ownership.
- **Generalize all startup operations together:** rejected; it would widen the
  slice beyond session-load callback binding.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. No MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, or manufacturer requirement applies. The mandatory embedded
assurance gate is `N/A`; the embedded workflow and simplifier were reviewed
for applicability and no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Aristotle the 4th / Luna max; bounded read-only wait timed out
  and the agent was closed. Status is `NO_CONCLUSION`; no child architecture
  PASS is claimed.
- Independent review: Darwin the 4th / Luna max; bounded read-only wait timed
  out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for typed contract, callback identity, baseline
  ordering, invalid-manifest retention, recovery continuation, exception
  propagation, and dependency direction.
- Simplification assessment: PASS. The existing session-load lifecycle owner
  is extended with no new state, coordinator, runner wrapper, or compatibility
  shim. Explicit success/failure closures keep callback binding readable.

## Verification target and limits

- Required and authorized here: source/dependency/order probe, inline valid,
  absent, invalid, failure, continuation, and exception dispatch probe,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, and expected release NO-GO evidence.
- Not proven: native TaskRunner timing, QApplication startup/session restore,
  filesystem durability, accessibility/DPI, clean-machine/cross-machine
  behavior, signing, installer, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
