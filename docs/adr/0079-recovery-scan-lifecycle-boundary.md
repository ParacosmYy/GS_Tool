# ADR-0079: Recovery Scan lifecycle boundary

- **Status:** accepted-with-limits; D54 / ARCH-44 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

Recovery inventory scans used a MainWindow boolean and callbacks that captured
only a `startup` flag. The TaskRunner operation ID was allocated but not part
of the callback identity contract, so a late success or failure callback had
no explicit stale guard before clearing scan state or continuing startup
restore.

## Decision

Add the Qt-free `presentation.recovery_scan_tracker.RecoveryScanTracker` and
immutable `RecoveryScanJob`. The tracker binds one positive operation ID and
the startup context, and MainWindow passes the same job object through both
TaskRunner callbacks. `finish()` releases only the current job; stale success
or failure callbacks return before candidate validation, notification, or
session restoration.

MainWindow remains responsible for RecoveryService, TaskRunner dispatch,
candidate type validation, candidate prompting, startup/session continuation,
notifications, and close guards. The tracker does not complete the generic
OperationTracker because recovery scans intentionally remain outside the
generic busy phase, preserving the existing behavior.

## Invariants

1. A duplicate scan is rejected without overwriting the active job.
2. Success and failure callbacks carry the typed job identity and must pass
   the finish guard before projecting any result or error.
3. The startup context is read from the current job, so a stale callback cannot
   advance a later startup or manual scan.
4. Invalid candidate results release the current job once and use the existing
   recovery-scan failure projection.
5. The tracker imports no Qt, RecoveryService, TaskRunner, candidate, session,
   notification, or close-policy type.

## Defect correction after independent review

The independent review found that the manual `file.recover` command could be
invoked while startup restoration was waiting. A manual scan could then occupy
the single tracker slot and cause the later startup scan to be rejected,
leaving the startup barrier uncleared. MainWindow now rejects manual recovery
requests during `_startup_restore_inflight` and reuses the existing localized
warning. The pending-startup policy remains in MainWindow; no queue or policy
state was added to the tracker.

## Alternatives considered

- **Keep the boolean and startup closure:** rejected because stale callbacks
  can only be distinguished implicitly by timing.
- **Use the generic busy OperationTracker:** rejected because recovery scan
  policy is intentionally a close-guarded background operation, not a generic
  document busy operation.
- **Move candidate validation or startup restore into the tracker:** rejected
  because those are application policy and user-facing projection owned by
  MainWindow.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or
firmware; MCU, BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and vendor-manufacturer requirements are not applicable.
Public CloudWeGo material remains transferable engineering reference only and
does not establish a private ByteDance standard, certification, or compliance
claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project references are `RecoveryService.scan_candidates`,
`TaskRunner`, `RecoveryCandidate`, `SessionRestoreTracker`, and the enterprise
architecture migration specification.

## Verification target and limits

- A Qt-free behavior probe covers duplicate begin, startup context, current/
  stale finish, and invalid operation ID.
- A source probe confirms the tracker is Qt-free, old boolean state is gone,
  both callback routes carry the same job, invalid-result failure projection
  remains, close uses the tracker, and the manual recovery startup guard is
  present.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D54 handoff.
- Native TaskRunner callback timing, recovery I/O, startup interaction,
  accessibility, DPI, clean-machine, cross-machine, signing, installer,
  updater, legal, support, and release-owner evidence remain unrun under the
  active no-launch and external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
