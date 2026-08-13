# ADR-0103: Plugin-host probe coordinator boundary

- **Status:** accepted-with-limits; D78 / ARCH-53 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` still contained the asynchronous diagnostic host probe start,
typed-result classification, and failure callbacks after catalog sequencing
was extracted. Those callbacks only coordinate a worker lifecycle and do not
own host protocol, containment, trust, or external-execution policy.

## Decision

Extract the host-probe sequencing into the Qt-free
`PluginHostProbeCoordinator`. It receives the existing `PluginHostClient`,
the shared `PluginOperationTracker`, a narrow task-submitter contract, and a
typed notification sink. It preserves the `host-probe` operation identity,
stale-completion guard, unavailable/in-flight/start/invalid/result/failure
notifications, and ready/rejected/error severity mapping.

Keep `MainWindow` responsible for composition, the shared plugin close-event
gates, notification ownership, and the application/infrastructure boundary
that enforces host protocol, containment, trust, and execution constraints.
The coordinator has no authority to load extensions or enable external
execution.

## Invariants

1. `presentation/plugin_host_coordinator.py` imports no PyQt6 or widget type.
2. The coordinator submits exactly the existing `PluginHostClient.probe`
   operation and accepts only `PluginHostProbeResult` at completion.
3. Stale `host-probe` callbacks cannot clear a newer operation.
4. MainWindow retains catalog, governance, and host close gates together.
5. Host protocol, containment, trust, execution-disabled, and security policy
   remain outside the presentation coordinator.
6. Shared task and notification Protocols have one Qt-free definition rather
   than domain coordinators importing each other for structural types.

## Alternatives considered

- **Leave host callbacks in MainWindow:** rejected; the host lifecycle remains
  coupled to the large Qt shell without owning any shell policy.
- **Move `PluginOperationTracker` into the coordinator:** rejected; close
  must gate catalog, governance, and host work through one shared tracker.
- **Import TaskRunner or a catalog coordinator Protocol:** rejected; the
  shared structural contracts keep both coordinators Qt-free and decoupled.
- **Move host protocol or execution policy into presentation:** rejected;
  that would broaden the UI boundary and weaken the existing safety model.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Anscombe the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Planck the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review: PASS for behavior-preserving extraction, Qt-free
  boundary, composition order, close-gate retention, and security-policy
  preservation.
- Simplification assessment: moving the three host callbacks reduces shell
  responsibility. Promoting the task and notification Protocols to shared
  Qt-free contracts removes duplicate structural definitions without moving
  policy. No further safe reduction was identified.

## Verification target and limits

- `D78-HOST-PROBE-BOUNDARY-PROBE=PASS` covers callback behavior tokens,
  MainWindow removal/wiring, composition order, and close-gate retention.
- `D78-COORDINATOR-QT-FREE-PROBE=PASS` confirms importing the coordinator and
  shared task contract through the bare Python path does not import PyQt6.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D78 handoff.
- The first probe failure was a probe-only source-order assumption about a
  command callback inside a method body; the probe was corrected and rerun.
  No source workaround was made for that false negative.
- Native Qt startup, callback interleaving, dialog rendering, accessibility,
  DPI, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
