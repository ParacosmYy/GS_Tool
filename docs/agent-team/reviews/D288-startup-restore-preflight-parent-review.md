# D288 parent review — startup restore preflight

## Scope

Reviewed `MainWindow.preflight_startup_restore`, the `DesktopRuntime` wrapper,
the `startup_restore_preflight` entry-point probe, and the static presentation
contract.

## Findings

- PASS — the probe reuses `restore_startup_state()` and therefore exercises the
  existing session-load, recovery-scan, document-open, and restore-projection
  chain rather than duplicating it.
- PASS — the wait is bounded by a monotonic five-second deadline and requires
  both the startup barrier and `TaskRunner` pending work to drain.
- PASS — synchronous and asynchronous failures enter the `finally` cleanup;
  recovery and session-save timers are stopped before the preflight returns.
- PASS — valid recovery candidates are detected before runtime construction and
  reported as a safe skip, so the no-window diagnostic cannot open a modal
  recovery decision surface.
- PASS — no `show()` or `exec()` is called, and normal `DesktopRuntime.start()`
  order is unchanged.
- PASS — simplification assessment found no smaller implementation that keeps
  lifecycle ownership in `MainWindow` while preserving the production path.

## Review limits

The independent Luna/max reviewer returned `NO_CONCLUSION` after two bounded
wait windows and was closed. This is recorded rather than presented as an
independent approval. Native EXE/Qt launch remains intentionally unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
