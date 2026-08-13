# D94 / ARCH-69 parent review: document-tab removal coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** The coordinator preserves the liveness guard and the
  former recovery, tab, editor, event, session, and empty-tab order.
- **Readability — PASS:** `_close_tab` remains the close-policy owner while
  finalization has one focused generic owner.
- **Architecture — PASS:** No Qt, editor, EventBus, RecoveryService, or
  concrete tab/capture type leaks into the coordinator.
- **Security/data safety — PASS:** Recovery snapshot cleanup and close event
  semantics are unchanged; no document content or persistence schema changed.
- **Performance — PASS:** No extra worker, copy, I/O, or event dispatch was
  introduced.

## Behavior review

1. Non-live tabs return before cancelling capture or scheduling cleanup.
2. Live capture is cancelled before snapshot cleanup, matching the old path.
3. Tab projection is removed before editor `deleteLater` and `DocumentClosed`.
4. Session save is requested after the close event and an initial document is
   created only after the tab surface becomes empty.
5. Dirty/save/cancel decisions and post-save removal callback remain in
   MainWindow.

## Simplification assessment

The extraction removes a mixed finalization block with one generic callback
contract and no new framework. No further safe behavior-preserving
simplification was identified.

## Review-role evidence

Lagrange the 3rd / Luna max architecture and Harvey the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D94 tab-removal boundary probe, targeted compileall, Ruff, format, package
identity, handoff, repository checks, and expected release no-go evidence are
required before delivery. Native close/save/recovery timing, runtime startup,
accessibility, clean-machine, cross-machine, and external release gates remain
unrun or open.
