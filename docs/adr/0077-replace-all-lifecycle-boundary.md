# ADR-0077: Replace All lifecycle boundary

- **Status:** accepted-with-limits; D52 / ARCH-42 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The active-document Replace All flow was represented by a MainWindow-owned
`_ReplaceAllJob`. Each cooperative slice was scheduled with a bare
`QTimer.singleShot` callback, so a callback that remained queued after
cancellation had no explicit identity to prove that it still belonged to the
same job. The flow also kept the expected document content version and
progress-report bookkeeping beside UI orchestration.

## Decision

Add the Qt-free `presentation.replace_all_tracker.ReplaceAllTracker` and its
typed `ReplaceAllJob`. The tracker owns only the active job identity, the
expected content-version guard, and progress-report state. Every queued slice
captures its job object; `is_current()` rejects a callback from an older job,
and `finish()` releases only that current identity. MainWindow remains the
owner of `ReplaceAllSession`, QTimer scheduling, editor locking, operation
busy/status policy, cancellation, rollback, dirty-marker restoration, and
user-facing messages.

## Invariants

1. A second job cannot overwrite an active job; MainWindow retains the
   duplicate/busy gate.
2. A queued callback must carry its `ReplaceAllJob` identity and pass the
   current-job guard before reading or mutating the editor session.
3. The expected content version is advanced only for the current job; a
   document change still cancels the session before the next slice.
4. Cancellation, limit completion, normal completion, and exception cleanup
   release only the matching job. Existing `OperationTracker` ownership and
   editor-session rollback remain unchanged.
5. The tracker imports no Qt, editor widget, service, timer, theme, locale,
   status, or notification type.

## Alternatives considered

- **Keep a bare job field and schedule a parameterless callback:** rejected;
  it cannot distinguish an old queued callback from a newly started job.
- **Move QTimer, session stepping, rollback, or status into the tracker:**
  rejected because those are editor and presentation policy owned by
  MainWindow/ReplaceAllSession.
- **Add a general async-operation coordinator or generation bus:** rejected as
  speculative; one opaque job identity is the complete current boundary.

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

The applicable project references are `EditorOperationPolicy`,
`ReplaceAllSession`, `OperationTracker`, and the enterprise architecture
migration specification.

## Verification target and limits

- A Qt-free behavior probe covers duplicate begin, current/stale identity,
  content-version update, finish release, and invalid operation IDs.
- A source probe confirms the tracker is Qt-free, the old MainWindow field is
  gone, QTimer callbacks carry a job identity, and stale/finish guards remain
  at the MainWindow boundary.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D52 handoff.
- Native Qt callback timing, editor rendering, accessibility, DPI,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  and release-owner evidence remain unrun under the active no-launch and
  external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
