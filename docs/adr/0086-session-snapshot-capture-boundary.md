# ADR-0086: Session-snapshot capture boundary

- **Status:** accepted-with-limits; D61 / ARCH-49 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow._build_session_snapshot()` combined three different concerns: Qt
editor reads, clean-tab eligibility, and ordered `SessionDocument` /
`SessionSnapshot` assembly. That made the session-save capture path harder to
reason about and made the pure metadata policy difficult to reuse or inspect
without a Qt coordinator.

## Decision

Add `presentation.session_snapshot_builder.build_session_snapshot[TabT]` as a
small framework-neutral assembly function. It accepts opaque tabs plus typed
callbacks for path, dirty state, modified state, and cursor position. The
function owns only:

- skipping pathless, dirty, or editor-modified tabs;
- preserving tab iteration order;
- converting valid cursor metadata into `SessionDocument` values;
- ignoring the existing `RuntimeError` / `ValueError` capture failures; and
- deriving the active document index from the retained ordered documents.

`MainWindow` remains the composition root. It still reads `EditorWidget`,
provides the active tab, owns session-save debounce and `SessionSaveTracker`,
invokes `SessionService` through `TaskRunner`, and retains startup, restore,
notification, and close policy. No document text, Qt object, service, timer,
or application policy crosses into the builder.

## Invariants

1. Callback evaluation preserves the previous path/dirty/modified short-circuit
   order.
2. Path-backed clean tabs retain their original order, and the active index is
   the first retained document whose path equals the active tab path; empty or
   unmatched snapshots use index `0`.
3. Cursor and `SessionDocument` validation failures remain local to the tab and
   do not abort the snapshot.
4. The builder imports no Qt module and owns no tab objects.
5. Session-save, recovery, startup, close, notification, and editor policy
   remain in MainWindow and existing services.

## Alternatives considered

- **Move the whole session-save flow into a service:** rejected; this would
  move Qt/editor reads, debounce, task execution, and application policy across
  a boundary that only needs pure metadata assembly.
- **Introduce a second snapshot DTO:** rejected; existing domain
  `SessionDocument` and `SessionSnapshot` already provide the stable contract.
- **Canonicalize paths in the new helper:** rejected; path normalization is not
  part of this slice and direct `Path` equality preserves existing behavior.

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

- Boundary and pure behavior probes cover Qt-free imports, callback shape,
  ordering, short-circuit filtering, invalid cursor handling, active-index
  assembly, and empty snapshots.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D61 handoff.
- Native Qt event timing, runtime startup, clean-machine, cross-machine,
  accessibility, and external release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
