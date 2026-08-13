# ADR-0323: Startup state preflight

- Status: accepted-with-limits
- Date: 2026-08-12
- Decision owners: Architect, product, developer, QA

## Context

The no-window `--diagnose-startup` path already proves package imports,
desktop composition, pre-show preparation, and one empty editor shell. It did
not yet expose whether the same user-local session and recovery manifests that
normal startup reads were valid. That left a real startup failure in the
asynchronous restore boundary difficult to distinguish from a missing native
runtime.

## Decision

Add two read-only probes at the existing application entry-point diagnostic
boundary:

- `session_preflight` calls the production `SessionService.load()` policy and
  reports only manifest path/presence, load state, document count, missing
  remembered-file count, and workspace-root presence.
- `recovery_preflight` calls the production
  `JsonRecoverySnapshotStore.list_snapshots()` adapter and reports only the
  recovery directory, manifest-file count, valid snapshot count, and derived
  invalid count.

The probes do not write settings, session, recovery, or document files; they do
not return document contents or a list of user document paths; and they remain
behind the explicit diagnostic switch. Normal `DesktopRuntime.start()` and the
asynchronous recovery-before-session restore order are unchanged.

## Consequences

The report can now identify a malformed session, missing remembered document,
or malformed recovery snapshot without requiring a GUI launch. A recovery
probe may parse existing snapshot payloads because that is the same storage
operation used by the production recovery scan; payload contents are not
serialized into the report. Native EXE startup, Qt event-loop behavior,
asynchronous completion, clean-machine behavior, and release acceptance remain
unproven under the no-launch policy.

## Public-source applicability

Python 3.12 standard-library `pathlib`/JSON behavior and Qt 6 lifecycle
references remain applicable background references. No manufacturer
requirement applies: this is Python/Qt desktop code, not embedded C/C++,
MCU/BSP/HAL/RTOS, or firmware.
