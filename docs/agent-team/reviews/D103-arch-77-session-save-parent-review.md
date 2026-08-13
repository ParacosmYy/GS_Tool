# D103 / ARCH-77 parent review: session-save request and dispatch boundary

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** `request_latest()` preserves the old request guard;
  `drain()` preserves single-flight admission and operation binding;
  `complete()`/`fail()` preserve stale, invalid, failure, notification, and
  queued-request behavior.
- **Readability — PASS:** The session-save lifecycle now reads in one
  coordinator from request through completion. Callback names make ownership
  explicit and the MainWindow retains a small concrete `_submit_session_save`
  adapter.
- **Architecture — PASS:** The coordinator imports only the domain snapshot,
  notification contract, and tracker. QTimer, SessionService, TaskRunner,
  editor/tab reads, startup state, and close policy remain in MainWindow.
- **Security/data safety — PASS by source:** No session schema, normalization,
  persistence format, filesystem path, or stored payload behavior changed.
  Failure still preserves the previous manifest and stale callbacks cannot
  release current work.
- **Performance — PASS:** No new worker, timer, I/O, allocation loop, or
  concurrent path was introduced; one callback layer replaces duplicate
  MainWindow branches.

## Contract/order review

The retained order is:

`request_latest → tracker.request → drain → next_operation_id → tracker.begin
→ submit_save → complete/fail → tracker.complete/fail → drain`.

The inline behavior probe covered a queued newer snapshot, invalid completion,
stale completion, and matching failure. Compileall, Ruff, format, and source
boundary probes also passed.

## Simplification assessment

`PASS`. Extending the existing coordinator is simpler than introducing a
second request coordinator. Removing `_queue_session_save()` and
`_drain_session_save()` eliminates duplicate lifecycle ownership without
moving concrete Qt/application effects. No further safe behavior-preserving
simplification was identified.

## Role evidence and limits

Wegener the 3rd / Luna max (architect) and Chandrasekhar the 3rd / Luna max
(independent review) both returned `NO_CONCLUSION` after bounded waits. No
child PASS is claimed. Native Qt startup, TaskRunner timing, durability,
clean-machine, and external release evidence remain open.
