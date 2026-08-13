# D97 / ARCH-71 parent review: document-open projection coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** The coordinator preserves ordinary open, ordinary
  duplicate, session duplicate, new-tab, line, cursor, restored-tab, event,
  notification, and continuation order.
- **Readability — PASS:** MainWindow composition shows the concrete Qt/editor
  callbacks, while one focused Qt-free module explains valid-open sequencing.
- **Architecture — PASS:** The module depends only on application/domain DTOs,
  `Path`, and typed callbacks; stale/invalid/failure classification remains in
  D86's `DocumentOpenCoordinator`.
- **Security/data safety — PASS:** No file access, path resolution, document
  mutation, persistence, plugin capability, or trust boundary changed.
- **Performance — PASS:** No copy, worker, I/O, retry, event bus, or new state
  model was added; the coordinator only invokes existing callbacks.

## Behavior review

1. Ordinary duplicate paths call the existing error projection and return
   before tab/event/notification work.
2. Session duplicate paths record the existing tab and continue the restore
   queue, matching the previous branch.
3. New tabs are added before line navigation; session cursor positioning and
   restored-tab recording happen before event and success notification.
4. Untitled results still notify as `document`; path and session coordinates
   are not normalized or rewritten.
5. D86 still consumes the matching session binding and rejects stale/invalid/
   failed callbacks before this coordinator is invoked.

## Simplification assessment

`PASS`. The extraction removes a 40-line valid-open policy block from
MainWindow and uses explicit callbacks rather than a service locator, event
bus, or broad document-operation abstraction. No further safe
behavior-preserving simplification was identified.

## Review-role evidence

Leibniz the 3rd / Luna max architecture and Dewey the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D97 projection and Qt-free wiring probes, compileall, Ruff, format,
package, traceability, handoff, repository, no-process, and expected release
NO-GO checks are required before delivery. Native editor/tab/session timing,
runtime startup, accessibility, clean-machine, cross-machine, and external
release gates remain unrun or open.
