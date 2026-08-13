# D50 parent review — recovery-capture lifecycle boundary

## Scope and decision

- **Delivery:** D50 / ARCH-40
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`RecoveryCaptureTracker` now owns the lifecycle indexes previously spread
across MainWindow: active document/write identity, opaque capture-job binding,
worker snapshot state, discarded callback IDs, and delete sequencing. The
tracker has no Qt or service dependency. MainWindow retains editor stepping,
channel backpressure, RecoveryService, TaskRunner, tab identity, notification
text, and close policy.

The required architecture consultation used Harvey the 2nd / Terra max because
the slice crosses cooperative capture, worker callbacks, cancellation, and
cleanup. Two bounded waits returned no conclusion; no architecture PASS is
claimed. The independent review used McClintock the 2nd / Terra max. Two
bounded waits returned no conclusion and the agent was closed; no independent
PASS is claimed.

## Source findings

- Duplicate document captures and duplicate snapshot bindings are rejected at
  the tracker boundary.
- `finish_capture()` removes only the UI producer; the document remains in
  flight until the worker callback path calls `complete_document()`.
- Aborting a worker-backed capture marks the snapshot discarded; the later
  success/failure callback consumes that marker exactly once.
- `finish_write()` hands deferred deletion back to MainWindow, and the tracker
  keeps one delete slot plus a pending request without moving the service call.
- MainWindow retains channel `offer/consume/finish/abort`, bounded UI slices,
  RecoveryService, TaskRunner, notifications, tab cleanup, and `closeEvent()`.
- The old `_recovery_channels` map had no read path; removing it is safe because
  the active job or TaskRunner operation retains the channel until completion.

## Simplification assessment

The extraction is deliberately generic only at the opaque job/owner boundary;
it does not introduce a recovery coordinator, callback framework, or second
service. Removing the write-only channel map reduces state without changing
ownership. The tracker API names each lifecycle transition directly, so the
MainWindow call chain is easier to inspect without hiding error or cancellation
policy. No new signal, timer, persistence schema, dependency, or test-only
asset was added.

## Authorized non-destructive validation

- D50 Qt-free tracker behavior probe — **PASS**: duplicate guard, identity,
  deferred delete, discarded callback, and abort release.
- D50 source/integration probe — **PASS**: Qt-free boundary, delegation,
  worker/discard/cleanup routes, and close guard.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `scripts\package.ps1` — **PASS**; root/dist portable candidates match at
  38,425,096 bytes with SHA-256
  `BA90DF4F4C8865B1FB1A8DA22D485ADCD934B9793173158301FC6D5BC3F5975E`.
- Handoff/check/release verification is recorded after documentation sync.
- Independent Terra review window — **NO_CONCLUSION** after two bounded waits;
  no independent PASS is claimed.
- No unit tests, mocks, fixtures, test-only assets, QApplication, Qt/EXE
  startup, screenshots, deployment, or hardware operation were created or
  run.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and vendor
requirements are not applicable. Public CloudWeGo sources remain engineering
references only; no private ByteDance standard, certification, or compliance
claim is made.

## Limits and disposition

Static evidence cannot prove native producer/worker timing, actual recovery
durability, power-loss behavior, runtime startup, or external release approval.
The bounded lifecycle state is accepted with those limits and remains subject
to the open runtime/release gates.
