# ADR-0102: Plugin catalog coordinator boundary

- **Status:** accepted-with-limits; D77 / ARCH-52 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` had accumulated the asynchronous extension-catalog scan and
descriptor approval/revocation callbacks alongside host probing, plugin
runtime controls, document operations, and close-event policy. The catalog
flow has a clear lifecycle owner but was coupled directly to Qt-shell members.

## Decision

Extract only catalog scan and descriptor governance sequencing into the
Qt-free `PluginCatalogCoordinator`. It accepts narrow Protocols for worker
submission, catalog view projection, and typed notification, and reuses the
existing `PluginOperationTracker`. MainWindow keeps the tracker itself because
the close event must gate catalog scan, catalog governance, and host probe
together. MainWindow also keeps host probing, runtime enablement/status,
command refresh, locale/theme application, and plugin trust/security policy.

The obsolete write-only catalog snapshot field is removed. No application
service contract, plugin trust boundary, external execution decision, signal,
or persistence behavior changes.

## Invariants

1. `presentation/plugin_catalog_coordinator.py` imports no PyQt6 or widget
   surface; Qt is reached only through injected structural contracts.
2. Catalog scan and governance operations retain their operation names,
   stale-completion guards, invalid-result checks, notification levels,
   governance enablement, and post-mutation rescan.
3. MainWindow retains the shared operation tracker and all close-event gates.
4. Catalog entries remain untrusted metadata and external code remains
   unloaded; no coordinator method grants execution authority.
5. The coordinator owns no locale, QSS, widget, filesystem, or persistence
   policy.

## Alternatives considered

- **Leave the callbacks in MainWindow:** rejected; the catalog lifecycle would
  remain coupled to the large Qt shell and harder to evolve independently.
- **Move the shared PluginOperationTracker into the coordinator:** rejected;
  MainWindow must gate catalog and host work together during close.
- **Pass concrete Qt surfaces and TaskRunner types:** rejected; narrow
  Protocols keep the extracted boundary Qt-free and make ownership explicit.
- **Keep the write-only catalog snapshot as a future cache:** rejected; it has
  no current consumer and would create a second, unobserved state owner.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Rawls the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Independent review: Sagan the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child PASS is claimed.
- Parent source review covers correctness, readability, architecture,
  security, and performance and found no required change.
- Simplification assessment: removing the write-only snapshot and unused
  refresh callback reduces state/indirection without changing behavior;
  Protocols are retained because they remove Qt coupling at a real boundary.

## Verification target and limits

- `D77-CATALOG-BOUNDARY-PROBE=PASS` covers MainWindow removal/wiring,
  coordinator contract presence, close-gate retention, and composition order.
- `D77-COORDINATOR-QT-FREE-PROBE=PASS` confirms importing the coordinator
  through the bare Python path does not import PyQt6.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D77 handoff.
- Native Qt startup, callback interleaving, dialog rendering, accessibility,
  DPI, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
