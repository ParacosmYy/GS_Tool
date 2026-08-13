# D52 parent review — Replace All lifecycle boundary

## Scope and decision

- **Delivery:** D52 / ARCH-42
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`ReplaceAllTracker` moves only the active job record and identity/content
version guard out of MainWindow. `ReplaceAllSession`, editor mutation, QTimer
slicing, operation busy state, cancellation, rollback, dirty restoration,
status, and notification policy remain where they were. Queued callbacks now
capture the job object, so a callback from a cancelled job cannot advance a
new job.

The required architecture consultation was attempted with Galileo the 2nd /
Terra max. Three bounded windows returned no conclusion; no architecture PASS
is claimed. The independent review was attempted with Zeno the 2nd / Terra
max. Two bounded windows returned no conclusion and the agent was closed; no
independent PASS is claimed.

## Source findings

- `begin()` rejects a second active job and validates the positive operation
  identity/content-version inputs.
- MainWindow schedules `partial(self._continue_replace_all, job)` and checks
  `is_current(job)` before every slice; stale queued callbacks return without
  touching the session or editor.
- Content-version updates and `finish()` are identity guarded, so stale jobs
  cannot release a newer lifecycle.
- Existing exception, cancellation, limit-exceeded, normal-completion,
  rollback, dirty-marker, editor-lock, tab-bar, find-surface, and generic
  operation cleanup paths remain in MainWindow.
- The tracker has no Qt or service dependencies and does not own UI/editor
  policy.

## Simplification assessment

The extraction is intentionally narrow. The tracker has one job record and
four lifecycle operations; it does not introduce a queue, signal, callback
bus, cancellation token, or general coordinator. Keeping the job identity in
the scheduled callback is necessary to make stale-queue behavior explicit.
Further inlining would restore the parameterless callback race, while moving
session/editor policy would increase coupling. No additional simplification
was justified after the code-simplification review.

## Authorized non-destructive validation

- `D52-replace-all-identity-stale-probe=PASS`.
- `D52-replace-all-qt-free-and-callback-boundary-probe=PASS`.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `scripts\verify_handoff.ps1` and `scripts\check.ps1` — **PASS** after
  documentation synchronization.
- Package identity and release verifier evidence are recorded after the D52
  package rebuild.
- Independent Terra review window — **NO_CONCLUSION** after two bounded
  waits; no independent PASS is claimed.
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

Static evidence cannot prove native Qt event-loop interleavings, actual editor
rollback durability, or runtime rendering. The bounded callback state is
accepted with those limits and remains subject to the open runtime/release
gates.
