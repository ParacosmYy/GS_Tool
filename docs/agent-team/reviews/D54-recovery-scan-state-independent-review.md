# D54 independent review — Recovery Scan lifecycle boundary

## Review status

- **Delivery:** D54 / ARCH-44
- **Reviewer:** Meitner the 2nd / Terra max
- **Mode:** read-only source review
- **Conclusion:** **REVISE** — required finding resolved before delivery close

The reviewer was asked to inspect duplicate scans, typed job/startup context,
stale success/failure callbacks, invalid candidate results, startup/session
restore, close guards, dependency direction, and abstraction size. Two bounded
The review found one required application-policy defect and otherwise found
the tracker boundary sound. The agent was read-only and closed without writing
files; the parent applied the correction and recorded the evidence below. This
file does not claim an independent PASS.

## Required finding — resolved

Manual `file.recover` was allowed while `_startup_restore_inflight` was true.
That manual `startup=False` scan could occupy the tracker slot before the
session-load callback scheduled the `startup=True` scan. The startup scan would
then be rejected, and the manual completion would not continue session
restore, leaving the startup barrier potentially stuck.

Correction: MainWindow's `_show_recovery_candidates()` now rejects manual
recovery during startup and projects the existing
`Restoring the previous session...` warning. No pending state was added to the
tracker.

## Parent evidence retained

- The tracker is Qt/service-free and does not own candidate or restore policy.
- Both TaskRunner callback routes carry the same job identity and stale
  callbacks cannot release or project a newer scan.
- The manual recovery startup guard keeps the startup scan as the only scan
  during the startup barrier.
- Qt-free behavior and source probes cover duplicate/current/stale paths;
  compile, Ruff, format, handoff, and package evidence are recorded
  separately.

## Correction evidence

- `D54-startup-manual-recovery-guard-probe=PASS`.
- `D54-tracker-and-close-policy-retained-probe=PASS`.
- `uv run python -m compileall -q src`, Ruff, format, handoff, check, and
  package identity remain passing after the correction.
- A separate Nash the 2nd / Terra max follow-up review was attempted after
  the correction; two bounded waits returned no conclusion and therefore
  provide no additional independent PASS.

## Required follow-up

Obtain authorized runtime recovery-scan/startup evidence before turning this
bounded `accepted-with-limits` record into a stronger runtime claim.
