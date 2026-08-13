# D91 / ARCH-66 parent review: recovery-capture abort coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** Matching capture cleanup preserves the previous
  identity guard, worker-started discard versus pre-worker release, channel
  abort, session cancellation, document release, and optional notification.
- **Readability — PASS:** The lifecycle block has one focused owner, while
  MainWindow keeps the scheduling and outcome decisions visible at the call
  sites.
- **Architecture — PASS:** The coordinator is Qt-free and generic over job and
  owner types. It owns no editor, service, filesystem, timer, or close policy.
- **Security/data safety — PASS:** No snapshot content, path, store, delete
  target, or recovery persistence behavior changed. Stale jobs remain unable to
  mutate current tracker state.
- **Performance — PASS:** No copy, worker, serialization, or I/O was added;
  the coordinator only invokes the existing cleanup seams.

## Behavior review

1. A stale or already-finished job returns before changing lifecycle state.
2. A worker-started failure marks the snapshot discarded so its later writer
   callback cannot publish it; a setup failure releases the snapshot instead.
3. A matching channel is aborted before the capture session and document
   lifecycle are released, preserving the existing wake-up and cleanup order.
4. Failure notification remains conditional on the caller's notify flag and a
   live owner; cancellation remains silent.
5. D90 writer callback failure still uses `_abort_recovery_write_capture` and
   does not get folded into this producer-side policy.

## Simplification assessment

The coordinator removes one mixed lifecycle block from MainWindow without
introducing a new framework, state machine, or abstraction layer. No further
safe behavior-preserving simplification was identified.

## Review-role evidence

Herschel the 3rd / Luna max architecture and Parfit the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D91 Qt-free boundary probe, targeted compileall, Ruff, format, package
identity, handoff, repository checks, and expected release no-go evidence are
required before delivery. Native capture/session/channel timing, runtime
startup, accessibility, clean-machine, cross-machine, and external release
gates remain unrun or open.
