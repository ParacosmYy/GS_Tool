# ADR-0319: Startup-error log lifecycle

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D283 / ARCH-253

## Context

The local startup log contained the historical D225 `_locale` failure even
after the source fix and subsequent package rebuild. A user or support
process reading that file could therefore attribute a current launch failure
to an old exception. The log is only written on an exceptional startup path,
so stale content has no success marker.

## Decision

At the start of the existing `quillforge.__main__.main()` entry boundary,
remove only the prior app-owned `startup-error.log` path using
`Path.unlink(missing_ok=True)`. Wrap the cleanup in a broad fail-open boundary:
permissions, a missing local-app-data path, or an unexpected path error must
not prevent startup. If the current attempt raises through the existing
`__main__` guard, `_record_startup_failure()` writes a fresh traceback after
the cleanup.

The cleanup is not recursive, does not touch settings/recovery/session files,
and does not run for arbitrary paths supplied by the user. It covers both the
frozen EXE entry and the project console-script entry because both call
`main()`.

## Consequences

Successful preflight or startup no longer leaves the prior failure log to be
misread as current evidence. A process that is killed after cleanup but before
the error handler may leave no log; this is preferable to presenting stale
evidence, and the next authorized run can collect a fresh report. Native
startup remains the authority for proving the full Windows path.

## Public-source applicability

Python 3.12 first-party `pathlib.Path.unlink` behavior is the applicable
reference: <https://docs.python.org/3/library/pathlib.html#pathlib.Path.unlink>.
No manufacturer requirement applies. This is not embedded C/C++, MCU,
BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware work; no certification
claim is made.

## Verification boundary

The startup-log lifecycle contract, presentation audit, Ruff, formatting,
compileall, source startup diagnostic, project checks, PyInstaller PE/archive
inspection, and package identity passed. The old local log was observed absent
after a successful source preflight. EXE/Qt launch, clean-machine startup,
signing, installer, updater, registry, and release-go evidence remain unrun or
open.
