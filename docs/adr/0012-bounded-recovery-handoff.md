# ADR-0012: Bounded recovery handoff state machine

## Status

Accepted for the current bounded recovery handoff implementation with explicit
support limits. The default application composition now uses the bounded path;
the evidence below does not establish a product-wide hard native-memory ceiling.

## Problem

The current recovery path reads position-safe chunks cooperatively, then holds
all chunks until the worker consumes them. This keeps the UI responsive and the
writer atomic, but it retains O(document) payload memory and cannot support a
hard memory ceiling.

## Decision

Introduce a bounded producer/consumer handoff with the following boundaries:

1. **Application port:** define a small `RecoveryChunkSink`/channel contract
   in `application.ports`. It owns no Qt types, widgets, file paths, or thread
   primitives in its public contract. The contract must support non-blocking
   `offer`, `finish`, `abort`, and a worker-side iterable consumer.
2. **Infrastructure implementation:** implement the bounded queue, capacity
   accounting, close state, and wake-up/error semantics in infrastructure.
   Queue capacity is measured in both chunk count and UTF-8 bytes so a single
   oversized chunk cannot bypass the bound.
3. **Presentation orchestration:** `MainWindow` starts the worker consumer
   before capture, passes a sink callback to `TextCaptureSession`, and pauses
   capture when `offer` reports backpressure. The UI must schedule a later Qt
   continuation; it must never block on a queue lock or wait for disk I/O.
4. **Editor adapter:** `TextCaptureSession` advances its Scintilla position
   only after the sink accepts the current chunk. It retains at most the one
   candidate chunk needed for the current offer and clears it on cancellation.
5. **Atomic writer:** the consumer remains the only owner of JSON encoding,
   flush/fsync, and `os.replace`. No final snapshot is visible before the
   existing atomic commit boundary succeeds.

## State machine

The coordinator must make these states observable:

`pending -> streaming -> backpressured -> streaming -> finalizing -> committed`

Terminal alternatives are `cancelled`, `stale`, or `failed`. Once
`finalizing` begins, producer cancellation cannot pretend that a committed
snapshot did not exist: a successful commit is handled by the existing
snapshot lifecycle, while a failed commit preserves the prior valid snapshot.

### Required transitions

- **Queue full:** `streaming -> backpressured`; the UI returns to the Qt event
  loop and resumes only after capacity is observable.
- **Content version changed/tab removed:** producer becomes `stale`, aborts the
  channel, clears its candidate chunk, and never publishes a new snapshot.
- **User/application cancellation:** producer becomes `cancelled`; the worker
  receives a typed cancellation and exits without an error notification.
- **Worker failure before commit:** channel becomes `failed`; producer stops,
  the old final snapshot remains authoritative, and temporary output is
  removed.
- **Worker success:** `finalizing -> committed` only after the atomic replace;
  newer content is handled by the current version comparison and next-cycle
  capture policy.
- **Window close:** close remains blocked while the coordinator is nonterminal;
  no thread is abandoned and no UI callback is invoked after owner teardown.

## Invariants

1. The Qt event loop is never blocked by queue capacity, worker joins, or disk
   I/O.
2. The UI retains no unbounded collection of accepted chunks; at most the
   configured queue bound plus one candidate chunk is live in the handoff.
3. A stale or cancelled producer cannot cause a later worker callback to clear
   a newer snapshot or document state.
4. The final snapshot is either the previous valid file or a fully flushed and
   atomically replaced new file; no partial file is treated as valid.
5. All terminal paths release the channel, task reference, UI lock, and
   recovery inflight markers exactly once.

## Implementation slices

1. Add the port and an infrastructure channel with deterministic state/error
   semantics; verify bounded count/bytes and cancellation without Qt.
2. Add a production writer consumer that preserves the current atomic JSON
   store and exposes commit/failure completion to the presentation boundary.
3. Adapt `TextCaptureSession` and `MainWindow` to non-blocking offer,
   backpressure continuation, stale-version cancellation, and close handling.
4. Measure source/offscreen and packaged diagnostic paths with queue occupancy,
   peak native memory, cancellation, stale, worker failure, commit, and
   repeated-run outcomes before changing any support statement.

## Verification disposition

The source/offscreen production handoff and the packaged diagnostic both cover
bounded queue capacity, worker consumption, atomic commit, stale cancellation,
round-trip, and temporary-file cleanup. The current default bound is 64 chunks
and 1 MiB of queued UTF-8 bytes; the capture adapter retains at most the active
candidate chunk in addition to that queue.

## Consequences

The design separates editor reads, application recovery policy, queue
coordination, and atomic persistence. It makes the difficult commit/cancel race
explicit and keeps the Qt event loop non-blocking. Clean-machine behavior,
larger-than-measured workloads, permission/disk pressure, hard-power
durability, cross-machine repeatability, and a product support memory ceiling
remain open gates.
