# D49 parent review — session-save state boundary

## Scope and decision

- **Delivery:** D49 / ARCH-39
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`SessionSaveTracker` extracts only the latest-wins persistence state from
MainWindow. The Qt-free tracker owns the saved baseline, queued latest
snapshot, in-flight flag, operation binding, and stale/invalid/valid callback
classification. MainWindow retains the timer, snapshot capture, startup
barrier, SessionService, TaskRunner, operation allocation, notifications, and
close policy.

The required architecture consultation was attempted with Anscombe the 2nd /
Luna max. Two bounded waits returned no conclusion; no architecture PASS is
claimed. The independent review was attempted with Averroes the 2nd / Luna
max. Two bounded waits returned no conclusion and the agent was closed; no
independent PASS is claimed.

## Source findings

- `request()` keeps the newest snapshot and skips only an already-persisted
  baseline when no request or save is active.
- `begin()` binds one positive operation ID only when a request is present and
  consumes the queued snapshot exactly once.
- `complete()` rejects mismatched callback IDs without disturbing current
  state; matching valid results advance the baseline, while invalid results
  release the callback state for the existing MainWindow error path.
- `fail()` releases only the matching callback and leaves a newer queued
  snapshot available for the existing drain path.
- MainWindow keeps the 250 ms QTimer debounce, startup suppression, service
  call, TaskRunner dispatch, notification text, and `closeEvent()` pending-work
  guard.
- The new tracker imports the domain snapshot only and has no PyQt6 import.

## Simplification assessment

This is the smallest complete state extraction: one Qt-free value object with
`request`, `begin`, `complete`, and `fail`, plus read-only lifecycle properties.
Moving the timer, service, runner, notification, or close policy would increase
coupling and make the boundary less reusable. No new signal, persistence
schema, thread, timer, service, coordinator, or test-only asset was added.

## Authorized non-destructive validation

- D49 session-save source probe — **PASS**.
- D49 latest-wins/stale-callback and invalid-result/failure-drain probe —
  **PASS**.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `scripts\package.ps1` — **PASS**; root/dist portable candidates match at
  38,422,095 bytes with SHA-256
  `C4AECB4E001C13786C451188E489A72BC5A4D3774DD88CED0BE919BFE87400B6`.
- `scripts\verify_handoff.ps1` and `scripts\check.ps1` — **PASS** after
  documentation synchronization.
- `scripts\verify_release_handoff.ps1` — **EXPECTED NO-GO**; the known open
  release gates remain recorded in the handoff.
- Independent Luna review window — **NO_CONCLUSION** after two bounded waits;
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

Static evidence cannot prove queued Qt callback timing, actual manifest I/O,
runtime startup, close-event interleavings, accessibility, DPI,
clean-machine/cross-machine behavior, or external release approval. The
bounded state boundary is accepted with those limits and remains subject to
the open release gates.
