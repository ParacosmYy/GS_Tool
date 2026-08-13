# D294 parent review — startup diagnostic Qt lifecycle

## Scope

Reviewed `_run_startup_diagnostic()`,
`_startup_runtime_composition()`, `_startup_restore_preflight()`, the normal
`app.main()` entry path, and the targeted static contract.

## Findings

- PASS — the diagnostic acquires one `QApplication` before the probe sequence,
  allowing both Qt-heavy probes to reuse the same instance.
- PASS — only a diagnostic-owned application is quit; an outer application is
  preserved for embedding or composed callers.
- PASS — cleanup is in `finally`, while the diagnostic remains no-window and
  does not enter `application.exec()`.
- PASS — normal desktop startup code is unchanged and still owns one
  application/event-loop lifecycle.
- PASS — the reproduced duplicate window-class warning disappeared from the
  source startup diagnostic output.
- PASS — the static audit requires the diagnostic application ownership,
  probe ordering, and post-probe cleanup boundary.

## Simplification assessment

PASS. A local diagnostic-owned reference and one `finally` boundary are the
smallest complete fix. A global singleton, platform override, helper-wide
signature rewrite, or production lifecycle abstraction would broaden coupling
without improving the reported diagnostic behavior.

## Architecture consultation and limits

The required Luna/max architecture window returned `NO_CONCLUSION` after three
bounded 60-second waits and was closed. No architecture PASS is claimed. Native
EXE startup, clean-machine behavior, and real interactive window lifetime were
not authorized or run. TaskRunner `BaseException` semantics remain a separate
slice by design.

## Applicability

Python/PyQt6 desktop code only. No embedded public-vendor requirement or
certification claim applies.
