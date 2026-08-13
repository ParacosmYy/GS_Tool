# D292 parent review — runtime shutdown activity boundary

## Scope

Reviewed `DesktopRuntime.stop()`, `MainWindow.stop_background_activity()`,
the existing `_stop_close_timers()` helper, `TaskRunner` ownership, the close
guard, and the targeted AST contract.

## Findings

- PASS — exceptional and normal runtime shutdown now stop periodic recovery
  and session-save activity before plugin deactivation.
- PASS — the new public port keeps timer ownership in `MainWindow`; the
  composition root does not reach through Qt implementation details.
- PASS — stopping timers is idempotent and does not change accepted close
  precedence or user-facing close dialogs.
- PASS — no forceful thread wait or callback suppression was introduced;
  queued `TaskRunner` work remains under its existing Qt ownership boundary.
- PASS — the AST audit targets `DesktopRuntime.stop()` and requires the
  activity-before-plugin order plus the presentation port implementation.

## Simplification assessment

PASS. The two-line lifecycle delegation is the smallest coherent fix. No
second timer abstraction, thread shutdown API, or broad exception wrapper is
warranted because each would expand ownership and alter asynchronous behavior.

## Consultation and limits

The architecture-role consultation returned `PASS_WITH_LIMITS`: the timer-only
port is accepted as the correct high-cohesion boundary, while worker teardown,
queued-callback draining, and runtime reuse remain explicitly out of scope.
Native EXE/Qt startup, clean-machine behavior, and worker teardown during a
real window close remain unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
