# ADR-0074: session-save state boundary

- **Status:** accepted-with-limits; D49 / ARCH-39 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` carried four related session-persistence fields: the last
accepted snapshot, a latest requested snapshot, an in-flight flag, and the
operation callback identity. That state was correct but easy to duplicate or
change independently while the coordinator continues to grow. The boundary
must preserve the 250 ms Qt debounce, latest-wins behavior, one in-flight
`SessionService.save()` call, TaskRunner callback identity, error messages,
startup restore baseline, and close-event waiting policy.

## Decision

Add the framework-neutral `presentation.session_save_tracker.SessionSaveTracker`.
It owns only the immutable `SessionSnapshot` baseline, the latest queued
snapshot, the positive TaskRunner operation ID, in-flight state, and matching
callback classification. `MainWindow` remains responsible for the Qt timer,
snapshot capture, startup barrier, `SessionService`, `TaskRunner`, operation-ID
allocation, notification projection, and close policy.

The tracker rejects stale callbacks without clearing a newer operation. A
matching valid result advances the saved baseline; a matching invalid result
releases the callback state for MainWindow's existing error notification; a
matching failure releases the callback state while preserving any newer
queued snapshot for the existing drain path.

## Invariants

1. A request is a `SessionSnapshot`; no Qt widget, service, filesystem, or
   notification dependency enters the tracker.
2. A snapshot equal to the saved baseline is skipped only when no request or
   save is pending; a request during an in-flight save remains latest-wins.
3. `begin()` binds exactly one queued snapshot to one positive operation ID
   and consumes only that queued value.
4. A callback whose ID does not match the current binding is `stale` and
   cannot clear or overwrite current state.
5. MainWindow retains debounce timing, startup suppression, save result/error
   text, TaskRunner pending-work semantics, and the immediate-save close path.

## Alternatives considered

- **Keep four fields in MainWindow:** rejected because the coupled lifecycle
  invariant remains implicit in the largest coordinator.
- **Move SessionService or TaskRunner into the tracker:** rejected because it
  would couple a reusable value-state boundary to infrastructure and Qt
  scheduling policy.
- **Introduce a general persistence coordinator:** rejected as a larger,
  speculative abstraction; this slice addresses one concrete state boundary.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or firmware;
MCU, BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and vendor-manufacturer requirements are not applicable. Public CloudWeGo
material remains transferable engineering reference only and does not
establish a private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project references are ADR-0036's composition boundary,
ADR-0072's Qt-free restore tracker, the enterprise architecture migration
specification, and the project-local UI/architecture skills.

## Verification target and limits

- A source probe proves the tracker is Qt-free, MainWindow delegates save
  state, guards precede operation allocation, and close policy remains in the
  coordinator.
- Compile, Ruff, format, handoff, package identity, and release no-go evidence
  are recorded in the handoff.
- Native callback timing, runtime startup, actual filesystem persistence,
  accessibility, DPI, clean-machine, cross-machine, signing, installer,
  updater, and release-owner evidence remain unrun under the active no-launch
  and external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created;
  the validation boundary is static and non-destructive.
