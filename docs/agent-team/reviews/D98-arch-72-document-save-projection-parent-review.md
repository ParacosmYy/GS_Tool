# D98 / ARCH-72 parent review: document-save projection coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** The valid result sequence remains state/clean editor,
  language, title, recovery cleanup, saved event, success notification, session
  save request, then the optional continuation. D87 still rejects stale,
  dead-tab, invalid, and failed callbacks before projection.
- **Readability — PASS:** MainWindow composition shows concrete Qt/editor,
  recovery, event, notification, and persistence callbacks; the new module
  names the valid-save ordering without hiding policy in a service locator.
- **Architecture — PASS:** The coordinator depends only on `DocumentState`,
  `Path`, and typed callbacks. It has no Qt or infrastructure dependency, and
  the presentation-to-application/domain direction is unchanged.
- **Security/data safety — PASS:** No file access, path resolution, target
  selection, document-service behavior, plugin capability, trust decision, or
  persistence schema changed; paths are passed through unchanged.
- **Performance — PASS:** No worker, I/O, copy, retry, cache, event-bus, or new
  state model was added. The new module only invokes the callbacks that the
  former method invoked.

## Behavior review

1. The state callback preserves the former `tab.state` then clean-editor order.
2. Language and title refresh still use the returned path and live tab.
3. Recovery cleanup still precedes `DocumentSaved` and success feedback.
4. Session-save scheduling still precedes `after`; an absent continuation is a
   no-op and exceptions are not swallowed.
5. D87 continues to own stale suppression, tab liveness, read-only release,
   invalid-result validation, failure projection, and close-safe operation
   classification.

## Simplification assessment

`PASS`. This is a focused extraction of one existing valid-save policy block,
not a universal document-operation abstraction. Explicit callbacks are clearer
than importing Qt/editor types or introducing a shared mutable context. No
further behavior-preserving simplification was identified without moving D87
policy back into the shell or collapsing two distinct responsibilities.

## Review-role evidence

Popper the 3rd / Luna max architect and Helmholtz the 3rd / Luna max
independent reviewer both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D98 source and order probes, compileall, Ruff, format, package, traceability,
handoff, repository, no-process, and expected release NO-GO checks are required
before delivery. Native editor/save timing, runtime startup, accessibility,
clean-machine, cross-machine, and external release gates remain unrun or open.
