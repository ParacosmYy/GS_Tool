# ADR-0170: Replace All admission boundary

- **Status:** accepted-with-limits; D131 / ARCH-108 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` still combined Replace All admission checks, editor session
creation, operation/tracker binding, and first-slice startup with its wider
Qt composition class. The cooperative QTimer loop, ReplaceAllTracker, and
ReplaceAllCompletionCoordinator already provided stable seams for the rest of
the lifecycle.

## Decision

Introduce the Qt-free `ReplaceAllAdmissionCoordinator` with frozen/slotted
`ReplaceAllAdmissionPorts`. The coordinator owns only:

- inflight/busy/active-tab/query admission;
- ReplaceAllSession creation through a caller-provided callback;
- operation ID and tracker job binding;
- safe duplicate-job cleanup; and
- handoff to a caller-provided starter.

MainWindow remains responsible for editor implementation, FindSurface and
tab-bar projection, QTimer scheduling, cooperative slices, progress,
cancellation, completion, rollback, and application policy.

## Invariants

1. An inflight job is rejected before busy or editor/session work.
2. Busy, empty tab/query, and `ValueError` branches preserve existing status
   text and warning level.
3. Session arguments, max-match policy, operation message, operation ID,
   content version, and dirty marker are captured in the same order.
4. A tracker race completes the newly reserved operation and starts no UI job.
5. A successful admission hands one job to the existing MainWindow starter;
   QTimer slicing and completion ownership do not move.
6. The coordinator imports no Qt, editor widget, surface, service, or global
   application state.

## Alternatives considered

- **Keep admission in MainWindow:** rejected; it leaves a complete policy
  cluster coupled to the largest composition root after tracker/completion
  seams already exist.
- **Move the whole Replace All loop:** rejected for this slice; it would mix
  Qt scheduling, editor slicing, rollback, and completion policy in one risky
  extraction.
- **Extract recovery capture first:** deferred; channel/backpressure and
  editor-slice lifecycle have a larger safety surface and should be a separate
  bounded slice.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation orchestration. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Arendt the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: James the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for branch precedence, session/operation/job identity,
  named port mapping, unchanged starter/loop/completion ownership, and Qt-free
  dependency direction.
- Simplification assessment: PASS. The coordinator extracts only admission
  and binding; no second tracker, event bus, or generic workflow framework was
  introduced.

## Verification target and limits

- Authorized evidence: admission behavior/contract probes, compileall, Ruff,
  format, project checks, package identity, root/dist identity, no-process
  evidence, handoff/register/index synchronization, and expected release
  NO-GO evidence.
- Not proven: native Find/Replace rendering, QTimer callback timing, editor
  slicing, filesystem durability, accessibility, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
