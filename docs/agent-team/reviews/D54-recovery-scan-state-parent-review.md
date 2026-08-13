# D54 parent review — Recovery Scan lifecycle boundary

## Scope and decision

- **Delivery:** D54 / ARCH-44
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`RecoveryScanTracker` replaces MainWindow's scan boolean with a typed job that
binds operation identity and startup context. Both TaskRunner callback routes
carry that job. MainWindow retains RecoveryService, TaskRunner, candidate
validation, candidate prompts, startup/session restore continuation,
notifications, and close policy.

The required architecture consultation was attempted with Banach the 2nd /
Terra max. Two bounded windows returned no conclusion; no architecture PASS is
claimed. The independent review was performed by Meitner the 2nd / Terra max
and returned **REVISE** with one required startup-order finding. The finding
was fixed before this delivery record was closed and is retained in the
independent review file; no independent PASS is claimed.

## Source findings

- `begin()` prevents a second scan from overwriting the current job and
  validates a positive operation ID.
- Success and failure callbacks pass the same immutable job object to
  MainWindow, and `finish()` rejects stale callbacks before any projection.
- The startup/manual branch remains on the job and still controls only the
  existing recovery/session continuation behavior.
- Invalid candidate results release the current job once and use the existing
  recovery failure notification path.
- `closeEvent()` checks tracker `inflight` state; the generic OperationTracker
  and background-worker policy remain unchanged.
- The tracker is Qt/service-free and owns no application or UI policy.
- `_show_recovery_candidates()` now rejects manual scans during startup restore,
  so the startup scan cannot be starved by a manual command.

## Independent finding and correction

The review identified a real race in the application policy: a manual recovery
command could start while `_startup_restore_inflight` was true, occupy the
single scan slot, and cause the startup scan to be dropped without continuing
session restore. The smallest correction is a MainWindow entry guard with the
existing restore warning. The tracker remains a one-job identity boundary and
does not gain pending-request policy.

## Simplification assessment

The tracker is the smallest complete extraction: one immutable job, one active
slot, begin/current/finish semantics, and no cancellation token, signal, queue,
or general async coordinator. Keeping invalid candidate projection in
MainWindow avoids duplicating recovery policy. Reusing OperationTracker would
make a background scan appear as generic document busy and change behavior, so
it was not a safe simplification.

## Authorized non-destructive validation

- `D54-recovery-scan-identity-stale-probe=PASS`.
- `D54-recovery-scan-qt-free-and-policy-boundary-probe=PASS`.
- `D54-startup-manual-recovery-guard-probe=PASS`.
- `D54-tracker-and-close-policy-retained-probe=PASS`.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- Package identity and release verifier evidence are recorded after the D54
  package rebuild.
- Independent Terra review — **REVISE**; the required startup-order finding
  was corrected and the guard/static probe passed. No independent PASS is
  claimed.
- Nash the 2nd / Terra max was assigned a bounded follow-up review after the
  correction; two bounded waits returned no conclusion, so no follow-up PASS
  is claimed.
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

Static evidence cannot prove native TaskRunner ordering, actual recovery
inventory I/O, or startup restore interaction. The bounded scan state is
accepted with those limits and remains subject to the open runtime/release
gates.
