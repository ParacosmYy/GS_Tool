# ADR-0205: plugin-runtime Ports contract

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D156 / ARCH-143

## Context

`PluginRuntimeCoordinator` already isolated plugin runtime status/control
projection, but its constructor accepted four independent presentation
callbacks. Runtime, trust, permission, and enablement policy were already
owned by application services while positional wiring made the presentation
boundary harder to review.

## Decision

Introduce the frozen/slotted Qt-free `PluginRuntimePorts` contract with named
callbacks for status view, busy predicate, command refresh, and notification.
Keep `PluginRuntime` as the concrete runtime dependency and preserve the
existing sequence:

- plugin failure: notify error, then refresh commands;
- unavailable runtime: notify error and return;
- busy operation: notify warning and return;
- runtime lifecycle exception: notify error and return;
- successful toggle: refresh commands, notify success, then show status.

MainWindow retains runtime, trust/permission/enablement policy, persistence,
plugin catalog/host boundaries, command registry, surface, close, and concrete
Qt ownership. No Qt type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable lifecycle-path wiring
  risk.
- Moving runtime policy, trust, permissions, or enablement into Ports would
  violate application ownership and security boundaries.
- Introducing an event bus or plugin state machine would add lifecycle
  complexity without changing behavior.

## Review and evidence

Maxwell the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Erdos the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes positional coupling
without adding behavior, policy, or a security layer.

Authorized non-destructive evidence:

- `D156-PLUGIN-RUNTIME-BRANCH-PROBE=PASS`
- `D156-FAILURE-UNAVAILABLE-BUSY-EXCEPTION-SUCCESS-PROBE=PASS`
- `D156-PORTS-IMMUTABILITY-PROBE=PASS`
- `D156-SOURCE-WIRING-PROBE=PASS`
- `D156-QT-FREE-CONTRACT-PROBE=PASS`
- `D156-PRESENTATION-AUDIT=PASS`
- `D156-COMPILEALL=PASS`
- `D156-RUFF=PASS`
- `D156-FORMAT=PASS`
- `D156-PACKAGE-BUILD=PASS`
- `D156-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `8393D4426D77E20AB5BD136B8E00D7E4ABAFD42248FD890F5159B36EE180DFB8`
- bytes: `38545918`
- source revision: `tree-sha256:3d2b2c24fe48c694c271e33e82514aad827fd7d2bfc39b6f11938720f60f5a32`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native plugin lifecycle
timing, runtime startup, clean-machine or cross-machine behavior, signing,
installer, update, legal clearance, support ownership, or release readiness.
Those gates remain open under the active no-launch/no-release authorization
boundary.
