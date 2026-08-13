# D291 parent review — entrypoint runtime cleanup

## Scope

Reviewed `quillforge.app.main()`, `DesktopRuntime.start()/stop()`, the plugin
manager lifecycle, and the targeted AST contract in
`scripts/audit_presentation_contracts.py`.

## Findings

- PASS — `runtime.start()` now shares the existing `try/finally` with
  `application.exec()`, so startup failures reach the existing idempotent
  `runtime.stop()` path.
- PASS — startup order and the composition boundary are unchanged; the patch
  does not add a second runtime abstraction or alter diagnostic dispatch.
- PASS — `PluginManager.deactivate_all()` iterates the current contexts and
  each `deactivate()` removes its context before invoking plugin cleanup,
  preserving safe repeated shutdown behavior.
- PASS — the AST audit targets `main()` and requires exactly one startup call,
  the event loop in the same `try` body, and cleanup in its `finally` body.

## Simplification assessment

PASS. The one-line control-flow movement is the smallest behavior-preserving
fix. No duplicate cleanup helper, broad exception catch, or new abstraction is
warranted; each would increase lifecycle surface without improving evidence.

## Limits

The architecture-role consultation returned `NO_CONCLUSION` after two bounded
wait windows and was closed. Native EXE/Qt startup, shell activation, and
clean-machine evidence remain intentionally unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
