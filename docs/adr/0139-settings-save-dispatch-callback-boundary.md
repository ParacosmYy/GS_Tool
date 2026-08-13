# ADR-0139: Settings-save dispatch callback boundary

- **Status:** accepted-with-limits; D112/ARCH-84 bounded architecture slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`SettingsSaveCoordinator` already owns settings-save callback classification:
tracker completion/stale suppression, `SettingsSnapshot` validation, invalid
result projection, and worker-failure projection. MainWindow still binds the
settings service operation directly to `TaskRunner.submit()` and passes the
coordinator callbacks at the composition site.

## Decision

Extend the existing Qt-free coordinator with typed
`SettingsSaveOperation`, `SettingsSaveSuccess`, `SettingsSaveFailure`, and
`SettingsSaveDispatcher` contracts plus a keyword-only `submit(...)` method.
The method invokes the existing `complete()`/`fail()` paths through a
structural dispatcher supplied by MainWindow.

MainWindow now passes the concrete settings operation and `TaskRunner.submit`
while retaining dialog editing, tracker admission, operation-ID allocation,
SettingsService, theme/font/locale/editor/motion projection, notifications,
persistence, and close policy. A synchronous dispatcher error propagates
without synthesizing completion or changing tracker state.

## Invariants

1. Tracker admission, stale suppression, invalid-result/failure projection,
   and close readiness remain unchanged.
2. Valid settings projection remains ordered as snapshot application,
   retranslation, editor-settings application, motion transition, and success
   notification.
3. The coordinator remains Qt-free and imports neither TaskRunner,
   SettingsService, QApplication, filesystem, nor widgets.
4. MainWindow retains settings UI editing, service/runner, persistence,
   theme/font/locale/motion policy, notification, and close ownership.

## Alternatives considered

- **Keep direct callback binding in MainWindow:** rejected; settings callback
  identity is already owned by the existing coordinator.
- **Create a second settings-dispatch coordinator:** rejected; it would split
  tracker classification from dispatch binding.
- **Move SettingsService, QApplication, or projection policy into the
  coordinator:** rejected; that would cross the Qt-free boundary.
- **Generalize all settings/document operations together:** rejected; it
  would widen the slice and obscure projection-order compatibility.

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

- Architect: Boole the 4th / Luna max; bounded read-only wait timed out and
  the agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS
  is claimed.
- Independent review: Bernoulli the 4th / Luna max; bounded read-only wait
  timed out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for typed contract, tracker identity, stale and
  invalid/failure order, projection ordering, exception propagation, and
  dependency direction.
- Simplification assessment: PASS. The existing settings lifecycle owner is
  extended with no new state, coordinator, runner wrapper, or compatibility
  shim. Explicit success/failure closures keep the callback boundary readable.

## Verification target and limits

- Required and authorized here: source/dependency/order probe, inline valid,
  invalid, failure, stale, tracker, and exception dispatch probe, compileall,
  Ruff, format, package identity, handoff/register/index synchronization, and
  expected release NO-GO evidence.
- Not proven: native TaskRunner timing, QApplication settings interaction,
  actual theme/font/locale/motion rendering, filesystem durability,
  accessibility/DPI, clean-machine/cross-machine behavior, signing,
  installer, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run under the active project policy.
