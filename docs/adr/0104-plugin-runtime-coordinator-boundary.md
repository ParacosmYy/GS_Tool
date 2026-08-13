# ADR-0104: Plugin runtime coordinator boundary

- **Status:** accepted-with-limits; D79 / ARCH-54 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` still owned registered-plugin failure notification, status dialog
projection, and enable/disable callback sequencing. The application-owned
`PluginRuntime` already defines the lifecycle and security contract; the
remaining shell code only adapts that contract to the plugin surface and
existing busy/command/notification policies.

## Decision

Extract runtime failure/status/enablement sequencing into the Qt-free
`PluginRuntimeCoordinator`. It receives an optional `PluginRuntime`, a typed
status view, a busy predicate, a command-refresh callback, and a typed
notification sink. It preserves the existing failure/refresh, unavailable,
busy-warning, exception, success, and status-projection behavior.

Keep MainWindow responsible for composition, the document-operation busy
predicate, notification ownership, and shared plugin operation close gates.
Keep `PluginRuntime` as the sole application boundary for trust, enablement,
activation, and runtime security policy. The coordinator cannot grant
external execution or bypass application policy.

## Invariants

1. `presentation/plugin_runtime_coordinator.py` imports no PyQt6 or widget
   type.
2. `PluginFailed` notification text and command refresh remain paired.
3. Missing runtime, busy document operation, supported runtime exceptions,
   success notification, and status refresh retain their existing semantics.
4. MainWindow retains the shared catalog/governance/host tracker and close
   gates; no operation policy is moved to the coordinator.
5. PluginSurface callbacks are lazy and resolve only after the coordinator is
   composed; no callback is invoked during PluginSurface construction.
6. The application `PluginRuntime` protocol remains the only runtime policy
   authority.

## Alternatives considered

- **Leave runtime callbacks in MainWindow:** rejected; the shell remains
  coupled to lifecycle projection and runtime control adaptation.
- **Move busy or close policy into the coordinator:** rejected; those are
  document/window policies and must remain visible at the shell boundary.
- **Pass PluginSurface or TaskRunner concrete Qt types:** rejected; a typed
  view keeps the coordinator framework-neutral and easy to replace.
- **Move trust/enablement policy into presentation:** rejected; it would
  duplicate or weaken the application runtime contract.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Epicurus the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Hegel the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review: PASS for behavior-preserving extraction, Qt-free
  boundary, callback initialization order, and policy retention.
- Simplification assessment: one focused coordinator removes five MainWindow
  methods while reusing the shared notification contract. The explicit view,
  busy, refresh, and runtime seams remain because each is a real ownership
  boundary. No further safe reduction was identified.

## Verification target and limits

- `D79-RUNTIME-BOUNDARY-PROBE=PASS` covers method removal/wiring, callback
  order, shared close gates, and runtime-policy retention.
- `D79-RUNTIME-COORDINATOR-QT-FREE-PROBE=PASS` confirms importing the
  coordinator through the bare Python path does not import PyQt6.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D79 handoff.
- Native Qt event timing, dialog rendering, accessibility, runtime startup,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
