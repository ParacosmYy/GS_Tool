# ADR-0036: Isolate the desktop composition root

- **Status:** accepted-with-limits; Phase 1 implementation
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`quillforge.app.main()` is both the command-line/diagnostic dispatcher and the
composition root. It creates every concrete persistence, workspace, recovery,
plugin, and presentation dependency in one function. The existing dependency
direction is sound, but the wiring function has too many reasons to change and
is difficult to review as the product grows.

Public CloudWeGo material emphasizes interface-oriented extension, separable
core/extension concerns, verified incremental evolution, and observable service
governance. Those principles are adapted here to a local PyQt application; no
private ByteDance standard is inferred.

## Decision

Create a top-level `quillforge.composition` module containing:

- `DesktopRuntime`, the lifecycle owner for the assembled `MainWindow` and
  built-in `PluginManager`;
- `build_desktop_runtime(application)`, the only concrete adapter selection and
  wiring function for the desktop path. Keep the Qt-free diagnostic adapter
  choice in the separate `quillforge.diagnostic_composition` boundary so
  diagnostic dispatch does not import the desktop composition root.

Keep `quillforge.app.main()` responsible for argument-mode dispatch,
`QApplication` creation, event-loop ownership, and final runtime shutdown. The
new module passes the same application services, ports, policies, and factories
to `MainWindow`; it does not add a service locator or runtime dependency
container.

## Consequences

### Positive

- Entry-point dispatch and dependency construction have separate reasons to
  change.
- Concrete infrastructure selection is discoverable in one named composition
  module and can be reviewed independently.
- Future profile/window composition can add a second explicit builder without
  changing application contracts.
- Plugin startup/shutdown ordering remains a named lifecycle contract.

### Negative / limits

- `composition.py` remains intentionally large in Phase 1; it is a composition
  root, not a new business layer.
- `MainWindow` remains a large presentation coordinator and is a later bounded
  migration target.
- Runtime startup, visual rendering, and cross-machine behavior are unrun under
  the project no-launch policy.

## Alternatives rejected

- A global service locator: hides ownership and makes tests/diagnostics harder
  to reason about.
- A third-party dependency-injection container: adds a runtime abstraction and
  failure modes without a current product need.
- A wholesale folder rewrite: high regression risk and no incremental evidence.
- Microservice/RPC decomposition: not applicable to a local desktop process.

## Verification

The implementation must preserve the existing static architecture checks,
compile/lint/format gates, package provenance, plugin lifecycle order, and
release no-go evidence. No unit-test-only assets or Qt startup are authorized by
the current project policy.
