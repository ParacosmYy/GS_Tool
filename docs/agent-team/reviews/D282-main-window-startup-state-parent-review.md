# D282 parent review — MainWindow startup-state initialization

## Scope

Reviewed the `_busy` initialization move in `MainWindow.__init__` and the
new `_audit_main_window_startup_state_contract` in the presentation audit.

## Findings

- PASS — `_busy` retains the same initial value (`False`) and all operation
  transitions remain unchanged.
- PASS — the default is now assigned beside `_startup_restore_inflight`,
  before any `is_busy` callback capture.
- PASS — the audit parses only `MainWindow.__init__` for the default assignment,
  so runtime `self._busy = False` releases are not mistaken for defaults.
- PASS — the audit remains Qt-free and does not construct widgets or execute
  callbacks.

## Simplification assessment

PASS. Moving one existing assignment is the smallest behavior-preserving
remedy. The AST check is scoped to the exact invariant; no new runtime
abstraction, placeholder coordinator, or callback indirection was added.

## Applicability and limits

Python 3.12 AST behavior is the applicable public reference. No manufacturer
requirement or embedded scope applies. The architecture and independent Luna/
max review windows returned `NO_CONCLUSION` after bounded waits; this parent
review does not convert those windows into a pass. Native EXE/Qt startup was
not run.
