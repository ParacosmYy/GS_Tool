# ADR-0082: Session-restore tab projection boundary

- **Status:** accepted-with-limits; D57 / ARCH-46 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` kept a private `_session_restore_tabs` list beside the session
restore state machine. The list was only the ordered output of restore handling:
already-open tabs and newly opened tabs were appended in snapshot order, then
the active snapshot path was selected with a first-tab fallback. Keeping that
projection list in the coordinator duplicated lifecycle cleanup and made the
restore boundary harder to extend.

## Decision

Make the existing framework-neutral `SessionRestoreTracker` generic over an
opaque tab record. It owns only the ordered restored-tab references and one
selection operation that receives a `path_of` callback. The callback keeps the
tracker independent of Qt, `EditorWidget`, and `_DocumentTab` details. The
tracker clears the projection at `begin()` and `finish()`, appends through
`record_restored_tab()`, and returns the canonical active-path match or the
first recorded tab through `select_restored_tab()`.

`MainWindow` records the same three existing restore outcomes—an already-open
tab, a newly opened tab, or an existing tab found while consuming the snapshot—
and delegates only the final target selection. It retains startup guards,
workspace barriers, session/document/recovery services, `TaskRunner`, open
dispatch, tab projection, notifications, initial-document policy, and close
behavior.

## Invariants

1. Tab records remain opaque to the tracker; no Qt or editor import is added.
2. Record order remains the order in which the serial restore flow handles
   snapshot documents.
3. A matching active path wins using the existing canonical `path_key()`
   normalization; the first recorded tab remains the fallback.
4. Empty restore output still leaves MainWindow responsible for creating the
   initial document when the tab surface is empty.
5. `begin()` and `finish()` clear projection references, while deferred-path
   history retains its existing behavior.
6. No service, asynchronous execution, notification, startup, or close policy
   moves into the tracker.

## Alternatives considered

- **Keep `_session_restore_tabs` in MainWindow:** rejected because restore
  output lifecycle and selection would remain a second state owner beside the
  tracker.
- **Create a second tab-projection tracker:** rejected for this bounded slice;
  the output belongs to the same restore lifecycle, and a generic opaque
  collection keeps the existing tracker cohesive without adding another
  coordinator or reset protocol.
- **Expose `_DocumentTab` or `DocumentState` from the tracker:** rejected
  because it would couple a Qt-facing record shape into a framework-neutral
  state boundary.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target and limits

- Source probes confirm the generic tracker contract, Qt-free imports, all
  restore record call sites, delegation, active-path/fallback selection, and
  begin/finish cleanup.
- Compile, Ruff, format, handoff, package identity, and expected release no-go
  evidence are recorded in the D57 handoff.
- Native Qt callback/event timing, runtime startup, accessibility, DPI,
  clean-machine, cross-machine, and release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
