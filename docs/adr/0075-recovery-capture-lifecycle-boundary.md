# ADR-0075: recovery-capture lifecycle boundary

- **Status:** accepted-with-limits; D50 / ARCH-40 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

`MainWindow` carried several coupled recovery-capture indexes: active
document IDs, opaque editor capture jobs, worker-owned snapshot IDs, discarded
callback IDs, and delete-in-flight/pending state. The implementation must
continue to support UI-sliced text capture, bounded channel backpressure,
worker callbacks, tab removal, stale/discarded writes, delete-after-write
ordering, error notifications, and close guards. Moving services or editor
objects would make the boundary less safe, not more reusable.

## Decision

Add the framework-neutral
`presentation.recovery_capture_tracker.RecoveryCaptureTracker[JobT, OwnerT]`.
It stores only opaque capture jobs/owners and lifecycle indexes. Its contract
covers:

- one active capture/write lifecycle per document and one snapshot binding;
- capture removal that can retain the document while a worker write completes;
- discarded callback classification and one-shot consumption;
- worker snapshot release with deferred delete requests; and
- one delete-in-flight slot with explicit pending-delete handoff.

`MainWindow` remains responsible for editor `TextCaptureSession` stepping,
`RecoveryChunkChannel` creation/finish/abort, RecoveryService calls, TaskRunner
operation allocation and callbacks, tab identity, notification text, dirty
policy, and close behavior. The former channel-retention dictionary was
redundant: the job or TaskRunner operation already retains the channel for its
authorized lifetime, so it is removed as part of the same behavior-preserving
simplification.

## Invariants

1. The tracker imports no Qt, editor widget, filesystem, service, or runner
   type; `JobT` and `OwnerT` are opaque presentation values.
2. A document cannot register two active captures, and one snapshot cannot be
   bound to two active capture jobs.
3. Finishing the UI producer removes its job but keeps the document and worker
   snapshot in flight until the matching application callback path releases
   them.
4. Aborting a producer releases its document; an already-started worker is
   marked discarded so its later callback can delete rather than publish.
5. A delete requested during a write or delete is retained as the latest
   pending request and is handed back only after the current delete succeeds.
6. MainWindow's error, notification, recovery, tab, TaskRunner, and close
   policies remain the sole application behavior owners.

## Alternatives considered

- **Keep all recovery sets and maps in MainWindow:** rejected because the
  cross-callback identity and deletion ordering remain implicit in the largest
  coordinator.
- **Move `_RecoveryCaptureJob` and editor/channel operations into the tracker:**
  rejected because it would couple the reusable lifecycle state to Qt/editor
  objects and backpressure policy.
- **Create a RecoveryService coordinator:** rejected as a larger application
  use-case rewrite; this slice only makes existing presentation lifecycle
  state explicit.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or firmware;
MCU, BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and vendor-manufacturer requirements are not applicable. Public CloudWeGo
material remains transferable engineering reference only and does not
establish a private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project rules are the enterprise architecture migration spec,
ADR-0072/0074's Qt-free state-boundary pattern, the recovery port contract,
and the project-local enterprise architecture skill.

## Verification target and limits

- Static source evidence proves the tracker contract, MainWindow delegation,
  worker/discard/cleanup routes, and close guard projection.
- A Qt-free behavior probe covers duplicate capture, stale identity, deferred
  deletion, discarded callback consumption, and abort release.
- Compile, Ruff, format, handoff, package identity, and release no-go evidence
  are recorded in the handoff.
- Native callback timing, actual recovery I/O, hard-power durability, runtime
  startup, accessibility, DPI, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner evidence remain
  unrun under the active no-launch/external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
