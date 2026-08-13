# ADR-0324: Bounded startup restore preflight

- Status: accepted-with-limits
- Date: 2026-08-12
- Decision owners: Architect, product, developer, QA

## Context

D287 verified the persisted session and recovery manifests but deliberately
stopped before the asynchronous restore chain. A packaged candidate could
therefore pass composition while still failing during worker completion,
session document opening, or the final restore barrier.

## Decision

Add `startup_restore_preflight` to the existing explicit no-window diagnostic.
The probe delegates lifecycle ownership to `MainWindow.preflight_startup_restore`
through a thin `DesktopRuntime` method. The production
`restore_startup_state()` path is invoked, Qt events are processed only until
the startup barrier and `TaskRunner` work drain or a five-second bound expires,
and recovery/session-save timers are stopped in `finally`.

If valid recovery candidates exist, the application entry-point reports an
explicit `skipped_recovery_candidates` result instead of invoking the modal
recovery decision surface. The probe never calls `show()` or `exec()`, and
normal `DesktopRuntime.start()` ordering is unchanged.

## Consequences

The source diagnostic now exercises session load, recovery scan, document file
open, editor-tab projection, and restore completion for the current no-recovery
state. It also proves that no worker work remains at the end of the probe. A
candidate with recovery snapshots still requires a user-owned interactive
decision and is reported as safely skipped rather than falsely marked as
restored.

## Public-source applicability

Python 3.12 `time.monotonic`/`try`-`finally` semantics and public Qt 6
`QApplication.processEvents`/lifecycle behavior are applicable references. No
manufacturer requirement applies: this is Python/Qt desktop code, not embedded
C/C++, MCU/BSP/HAL/RTOS, or firmware.
