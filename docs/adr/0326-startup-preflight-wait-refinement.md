# ADR-0326: Bounded startup preflight wait refinement

- Status: accepted-with-limits
- Date: 2026-08-12
- Decision owners: Architect, product, developer, QA

## Context

D289 proved that a regular file reaches the production document-open boundary,
but its synchronous no-window completion helper repeatedly called
`QApplication.processEvents()` while asynchronous work was pending. On the
current machine, one source file-open diagnostic needed 8,679 event batches in
about 592 ms. The loop was correct but needlessly busy when the Qt queue was
temporarily empty.

## Decision

Keep the completion wait in `MainWindow` and add an unparented, short-lived 2ms
`QTimer` as an explicit wake source. Process `AllEvents | WaitForMoreEvents`
until the existing monotonic deadline and the existing restore/task/path
predicates are clear. Stop the wake timer in `finally`; releasing the local
unparented timer avoids accumulating stopped child objects across repeated
preflights. The diagnostic still never calls `QApplication.exec()` or shows a
window, and the normal `DesktopRuntime.start()` path is unchanged.

Qt 6.11 documents `WaitForMoreEvents` as waiting when no pending events are
available. Its event-loop reference also documents the process-events flags;
the project uses that public API rather than an arbitrary sleep or a nested
`QEventLoop.exec()`.

## Timeout semantics

The timeout is explicitly `soft`: the helper checks the monotonic deadline
before and after each event-processing call, but Qt cannot interrupt an event
handler already executing on the main thread. The report therefore includes
`"timeout_mode": "soft"` so consumers do not mistake the diagnostic bound for
a hard scheduling guarantee.

## Consequences

Repeated source diagnostics now complete with 7–9 event batches while retaining
`startup_paths_open=1`, `pending_work=false`, and the no-window fields. The
2ms wake interval is intentionally conservative and diagnostic-only; native
startup latency and frozen-process behavior remain unverified.

## Public-source applicability

Python 3.12 `time.monotonic`/`try`-`finally` behavior and Qt 6.11 public
`QEventLoop::ProcessEventsFlag`/`processEvents` documentation apply. Source:
<https://doc.qt.io/qt-6/qeventloop.html>. No manufacturer requirement applies:
this is Python/Qt desktop code, not embedded C/C++, MCU/BSP/HAL/RTOS, or
firmware.
