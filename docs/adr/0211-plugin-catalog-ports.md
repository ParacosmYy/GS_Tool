# ADR-0211: Plugin-catalog Ports contract

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D162 / ARCH-149

## Context

`PluginCatalogCoordinator` had a positional constructor carrying the optional
catalog service, optional approval service, operation tracker, task submitter,
catalog view, and notification sink. The coordinator already keeps metadata
validation and governance policy outside the Qt layer, but positional assembly
made the seam order-sensitive as catalog capabilities grow.

## Decision

Introduce frozen/slotted `PluginCatalogPorts` and construct the coordinator with
that one named contract. Preserve the existing behavior and ownership:

1. an absent catalog emits the existing error and submits no scan;
2. an active scan emits the existing warning and submits no duplicate work;
3. a new scan begins tracking, emits info, and submits `catalog.scan`;
4. only the matching scan operation completes; valid snapshots notify their
   summary with the existing warning/success severity and project to the view;
5. absent governance, invalid targets, and active scan/governance retain their
   existing error/warning guards;
6. governance disables actions, emits info, submits the existing approve or
   revoke operation, and on success emits success before rescanning; failures
   reenable actions and emit error; stale operation IDs remain silent.

`MainWindow` retains the catalog service, approval ledger, execution gate,
operation tracker, TaskRunner, notification, plugin surface, and security policy
ownership. The Ports contract does not authorize external code execution or
change plugin loading behavior.

## Alternatives considered

- Keep positional constructor arguments: rejected because scan/governance
  dependencies remain order-sensitive and hidden at the composition root.
- Move catalog or approval policy into the coordinator: rejected because it
  would couple presentation to metadata, ledger, and execution policy.
- Replace services with untyped callables: rejected for this bounded slice
  because the existing application service and typed snapshot seams are clear.

## Review and simplification

- Architect role: Aristotle the 5th / Luna max; bounded waits ended without a
  conclusion, recorded as `NO_CONCLUSION`.
- Independent review: Sagan the 5th / Luna max; bounded review window is
  recorded as `NO_CONCLUSION` unless a later result is received.
- Parent review: PASS for the minimal source change, branch/order preservation,
  and MainWindow ownership.
- Simplification assessment: PASS; one immutable contract removes positional
  assembly without adapters, duplicate state, or policy movement.

## Authorized evidence

- Inline production-class scan/governance branch/order/stale/immutability
  probes: `D162-PLUGIN-CATALOG-BRANCH-PROBE=PASS`,
  `D162-PLUGIN-CATALOG-ORDER-STALE-GUARD-PROBE=PASS`,
  `D162-PORTS-IMMUTABILITY-PROBE=PASS`.
- Source and Qt-free AST probes: `D162-SOURCE-WIRING-PROBE=PASS`,
  `D162-QT-FREE-CONTRACT-PROBE=PASS`.
- `scripts/check.ps1`: `D162-PRESENTATION-AUDIT=PASS`,
  `D162-COMPILEALL=PASS`, `D162-RUFF=PASS`, `D162-FORMAT=PASS`.
- Package and identity evidence are recorded after the D162 package build.
- Release handoff remains expected `NO-GO`; no application launch, clean-machine
  run, flashing, deployment, or target operation was authorized.

## Public-source applicability

This is Python 3.12/PyQt6 presentation architecture. Embedded C/C++,
MCU/RTOS/BSP/HAL, and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only; no private ByteDance
standard or compliance claim is made.

## Limits

This decision does not prove metadata filesystem timing, approval-ledger
durability, plugin security containment, cross-machine behavior, packaged
startup, signing, installer/update behavior, or enterprise release readiness.
Those gates remain open in the release dossier.
