# ADR-0090: Document-tab forwarding simplification

- **Status:** accepted-with-limits; D65 / ARCH-50 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` retained four private methods that only forwarded to the existing
`DocumentTabSurface` contract: active-tab lookup, editor lookup, path lookup,
and tab containment. The wrappers added coordinator surface area without
owning policy, while callers already operated in the tab-surface boundary.

## Decision

Remove `_active_tab()`, `_find_tab()`, `_find_tab_by_path()`, and
`_contains_tab()` from MainWindow. Callers use the existing surface methods
directly:

- `active_tab()` for active document projection;
- `find_by_editor()` for editor callbacks;
- `find_by_path(..., exclude=...)` for path identity and duplicate guards; and
- `contains()` for stale callback/tab-lifecycle guards.

No new abstraction is introduced. MainWindow continues to own every save,
open, close, recovery, Replace All, Find, session-restore, status, and
application policy decision.

## Invariants

1. `DocumentTabSurface` remains the sole owner of tab registry lookup and path
   identity semantics.
2. Save duplicate detection retains the `exclude=tab` behavior.
3. Session restore keeps its ordered subset policy; direct lookup does not
   replace `SessionRestoreTracker` selection logic.
4. Contains checks remain at every callback/lifecycle guard before touching a
   tab or editor.
5. No Qt widget, service, TaskRunner, notification, close, or document policy
   moves in this simplification.

## Alternatives considered

- **Keep wrappers for readability:** rejected; they had no policy or semantic
  name beyond the existing surface contract.
- **Create a new TabLookupService:** rejected; it would duplicate the existing
  cohesive DocumentTabSurface registry boundary.
- **Move session restore policy into DocumentTabSurface:** rejected; restore
  subset/ordering remains application policy in MainWindow and
  SessionRestoreTracker.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-coordinator code.
Embedded C/C++, MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power, motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made.

## Verification target and limits

- Source probes confirm wrapper removal, direct surface calls, path exclusion,
  and session-snapshot active-tab retention.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D65 handoff.
- Native Qt callback timing, runtime startup, clean-machine, cross-machine,
  accessibility, and external release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
