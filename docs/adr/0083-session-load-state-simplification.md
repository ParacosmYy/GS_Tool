# ADR-0083: Remove write-only session-load coordinator state

- **Status:** accepted-with-limits; D58 / ARCH-47 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` initialized `_session_load_state` and assigned either the
`SessionLoadResult.state` value or `"invalid"` in the session-load callbacks,
but no code path read the field. The actual load result still controls invalid
notification projection, snapshot fallback, save baseline, and the recovery-
before-restore sequence directly.

## Decision

Remove the write-only field and its three assignments. Keep all existing
session-load behavior: invalid result type and failed load use
`DEFAULT_SESSION`, preserve the original-manifest error notification, bind the
default snapshot and save baseline, and schedule startup recovery scanning;
valid results continue to use the returned state for invalid notification
classification and the returned/default snapshot for restore.

No replacement tracker or state enum is introduced because there is no
consumer-owned lifecycle state to move. This is a behavior-preserving
coordinator simplification, not a session-service or restore-policy rewrite.

## Invariants

1. No read or write of `_session_load_state` remains.
2. `SessionLoadResult.state == "invalid"` still projects the existing error.
3. Invalid result shapes and failed loads still use `DEFAULT_SESSION` for both
   restore and last-saved baseline.
4. Recovery scanning remains scheduled with `startup=True` after every load
   outcome.
5. Startup guard, TaskRunner operation identity, close behavior, and session
   restore policy remain unchanged.

## Alternatives considered

- **Retain the field for future use:** rejected because write-only state adds a
  false lifecycle owner and cannot provide observability without a consumer.
- **Create a SessionLoadTracker:** rejected because no state transition or
  stale-callback contract remains after the dead field is removed; a new
  tracker would add coupling and reset complexity.
- **Change SessionLoadResult or SessionService:** rejected; the existing
  application contract already carries the state needed by current policy.

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

- A source reachability probe confirms the field has no remaining references
  and that invalid/default/recovery semantics remain in the callbacks.
- Compile, Ruff, format, handoff, package identity, and expected release no-go
  evidence are recorded in the D58 handoff.
- Native Qt callback timing, runtime startup, clean-machine, cross-machine,
  and release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
