# ADR-0210: Plugin-host probe Ports contract

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D161 / ARCH-148

## Context

`PluginHostProbeCoordinator` had a positional constructor carrying the optional
host client, plugin-operation tracker, task runner, and notification sink. The
coordinator is already Qt-free, but positional assembly made the seam less
discoverable and easier to wire incorrectly as plugin diagnostics evolve.

## Decision

Introduce frozen/slotted `PluginHostProbePorts` and construct the coordinator
with that one named contract. Keep the existing behavior and ownership:

1. an absent host emits the existing error and submits no task;
2. an in-flight `host-probe` emits the existing warning and submits no task;
3. a new probe begins the operation, emits the existing info message, then
   submits `host.probe` through the task boundary;
4. completion accepts only `PluginHostProbeResult`, clears only the matching
   operation ID, and maps ready/rejected/other states to success/warning/error;
5. failures clear only the matching operation ID and emit the existing error.

`MainWindow` retains the concrete host client, tracker, task runner,
notification, process-containment, and external-execution policy ownership.
The Ports contract does not authorize plugin loading or alter host security.

## Alternatives considered

- Keep positional constructor arguments: rejected because the dependency seam
  remains order-sensitive and obscures the coordinator boundary.
- Move host probing or containment into the coordinator: rejected because it
  would increase policy ownership and couple presentation to process details.
- Replace the host client with an untyped callable: rejected for this bounded
  slice because the existing `PluginHostClient` protocol and typed result are
  already the application seam.

## Review and simplification

- Architect role: Tesla the 5th / Luna max; bounded waits ended without a
  conclusion, recorded as `NO_CONCLUSION`.
- Independent review: Feynman the 5th / Luna max; bounded review window is
  recorded as `NO_CONCLUSION` unless a later result is received.
- Parent review: PASS for the minimal source change, exact branch/order
  preservation, and MainWindow ownership.
- Simplification assessment: PASS; one immutable contract removes positional
  assembly without introducing an adapter, new policy, or duplicate state.

## Authorized evidence

- Inline production-class branch/order/stale/immutability probe:
  `D161-PLUGIN-HOST-BRANCH-PROBE=PASS`,
  `D161-PLUGIN-HOST-ORDER-STALE-GUARD-PROBE=PASS`,
  `D161-PORTS-IMMUTABILITY-PROBE=PASS`.
- Source and Qt-free AST probes: `D161-SOURCE-WIRING-PROBE=PASS`,
  `D161-QT-FREE-CONTRACT-PROBE=PASS`.
- `scripts/check.ps1`: `D161-PRESENTATION-AUDIT=PASS`,
  `D161-COMPILEALL=PASS`, `D161-RUFF=PASS`, `D161-FORMAT=PASS`.
- Package and identity evidence are recorded after the D161 package build.
- Release handoff remains expected `NO-GO`; no application launch, clean-machine
  run, flashing, deployment, or target operation was authorized.

## Public-source applicability

This is Python 3.12/PyQt6 presentation architecture. Embedded C/C++,
MCU/RTOS/BSP/HAL, and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only; no private ByteDance
standard or compliance claim is made.

## Limits

This decision does not prove native worker timing, process containment,
cross-machine behavior, packaged startup, signing, installer/update behavior,
or enterprise release readiness. Those gates remain open in the release dossier.
