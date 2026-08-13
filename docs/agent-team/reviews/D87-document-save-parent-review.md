# D87 / ARCH-62 parent review: document-save coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** The coordinator calls generic completion first,
  ignores stale callbacks, gates removed tabs, restores editability before
  result validation, and delegates only valid `DocumentState` results.
- **Readability — PASS:** Save lifecycle concerns have one owner; MainWindow's
  valid saved-state consequences remain a small, explicit method.
- **Architecture — PASS:** `DocumentSaveCoordinator[TabT]` is Qt-free and
  opaque-tab based. It has no DocumentService, filesystem, editor surface,
  event bus, recovery, or close authority.
- **Security/data safety — PASS:** No save target, encoding, revision, or store
  operation changed. Existing duplicate path and service conflict policy stays
  in MainWindow/application code.
- **Performance — PASS:** No worker, copy, retry, or additional I/O was added;
  callback order and `after` timing remain unchanged.

## Behavior review

1. `_start_save` still checks busy/startup and duplicate path identity, snapshots
   state/text, locks the editor, begins the generic operation, and submits the
   same `DocumentService.save_document` call.
2. Current success/failure callbacks complete the generic operation before
   tab liveness. Stale callbacks cannot unlock a newer tab operation.
3. A removed tab is ignored without dereferencing its editor, matching the
   former callback behavior.
4. A live callback releases read-only before `DocumentState` validation. An
   invalid result preserves `Save failed` and the existing service error.
5. A live failure releases read-only and preserves `Save failed` with the
   worker exception text.
6. Valid saved state still updates tab state and language/title, clears the
   recovery snapshot, publishes `DocumentSaved`, notifies success, requests a
   session save, and invokes `after` only after all prior consequences.

## Simplification assessment

The generic opaque-tab seam removes duplicated operation/liveness/mutability
guards without hiding document policy. Save remains separate from open because
its state and editor-mutation contract is materially different. No further safe
simplification was identified.

## Review-role evidence

Kuhn the 3rd / Luna max architecture and Averroes the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D87 Qt-free boundary probe, targeted compileall, Ruff, format, package
identity, handoff, repository checks, and expected release no-go evidence are
required before delivery. Native tab/editor timing, runtime startup,
accessibility, clean-machine, cross-machine, and external release gates remain
unrun or open.
