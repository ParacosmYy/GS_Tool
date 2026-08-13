# D289 parent review — file-open preflight

## Scope

Reviewed the explicit file diagnostic in `app.py`, the
`DesktopRuntime.preflight_startup_paths()` seam, the MainWindow queued-path
wait, and the static file-open contract.

## Findings

- PASS — the diagnostic validates a regular file and reuses the production
  startup-path queue; it does not create a second document-open worker.
- PASS — runtime ordering remains preparation → restore → command refresh →
  queued startup path, with window display and `exec()` excluded.
- PASS — file opening is still performed by `DocumentOpenAdmissionCoordinator`
  and `DocumentService` through the existing `TaskRunner` boundary.
- PASS — recovery candidates are detected before the no-window path can reach a
  modal prompt, and the result is reported as an explicit safe skip.
- PASS — MainWindow timer cleanup covers both synchronous and asynchronous
  failures after the queue is submitted.
- PASS — simplification assessment found no smaller implementation that would
  preserve the existing startup-path ownership and completion evidence.

## Review limits

The independent Luna/max reviewer returned `NO_CONCLUSION` after two bounded
wait windows and was closed. Native shell activation and frozen EXE launch
remain intentionally unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
