# ADR-0167: Core command registration boundary

- **Status:** accepted-with-limits; D128 / ARCH-105 bounded application slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` still constructed and registered all 23 built-in commands inside
its large Qt composition class. `CommandSurface` already owned the Qt menu and
toolbar projection, but command catalog assembly remained mixed with window
lifecycle, plugin coordinators, and document/workspace callbacks.

## Decision

Introduce the Qt-free `CoreCommandCoordinator` and frozen/slotted
`CoreCommandPorts`. The coordinator owns only the deterministic built-in
command catalog and registers it into the injected `CommandRegistry`.
`MainWindow` maps its existing behavior callbacks by name and retains all
document, workspace, settings, plugin, close, and lifecycle policy. The
existing `CommandSurface` remains the sole menu/toolbar projection owner.

## Invariants

1. The 23 core command IDs retain their established order, titles, shortcuts,
   menu IDs, and callback identities.
2. `CommandRegistry` collision behavior is unchanged; the coordinator does
   not add idempotence, replacement, plugin policy, or global state.
3. Plugin commands continue to register through the existing registry and
   `CommandSurface.refresh()`/`refresh_command_menus()` path.
4. The new module imports only application command types and standard-library
   typing/dataclass facilities; it does not import Qt, widgets, services,
   EventBus, or a service locator.
5. `MainWindow` remains the callback composition and application-policy owner;
   `CommandSurface` remains the Qt projection owner.

## Alternatives considered

- **Keep the command tuple in MainWindow:** rejected; it leaves a stable
  application catalog coupled to the largest presentation orchestrator.
- **Move command construction into CommandSurface:** rejected; that would
  couple Qt projection to application behavior and plugin policy.
- **Use a mapping or generic registry factory:** rejected; named frozen ports
  make the callback boundary explicit and preserve extensibility without
  hiding required commands behind string keys or global state.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop application/presentation orchestration.
MCU, embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA,
Flash/NVM, power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Maxwell the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Bacon the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for exact command catalog coverage/order, named callback
  mapping, Qt-free dependency direction, unchanged registry semantics, and
  unchanged CommandSurface/plugin refresh ownership.
- Simplification assessment: PASS. One focused coordinator and one frozen
  ports contract remove the catalog from MainWindow without introducing a
  second command policy or abstraction layer.

## Verification target and limits

- Authorized evidence: exact core-command behavior/contract probes, compileall,
  Ruff, format, project checks, package identity, root/dist identity,
  no-process evidence, handoff/register/index synchronization, and expected
  release NO-GO evidence.
- Not proven: native QAction/menu/toolbar rendering, QApplication startup,
  runtime shortcut delivery, plugin interleaving, DPI/font metrics,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
