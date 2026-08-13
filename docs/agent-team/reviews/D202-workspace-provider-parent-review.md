# D202 parent review: workspace provider directory capability

## Decision

`PASS` for the bounded application/infrastructure contract slice, accepted
with explicit runtime and release limits.

## Review evidence

- `WorkspaceProvider` now owns the directory-capability predicate through one
  typed method, and `FileWorkspaceProvider` is the only implementation in the
  checkout.
- `WorkspaceService.open_workspace()` and `list_directory()` no longer call
  `.is_dir()` directly; they retain path normalization, containment, limits,
  and the prior error strings.
- Composition wiring is unchanged, so no provider implementation or runtime
  registration path was omitted.
- The adapter preserves the previous `Path.is_dir()` true/false semantics;
  enumeration remains in the same infrastructure adapter.

## Simplification assessment

`PASS`: adding one capability to the existing port is the smallest cohesive
change. A new filesystem service, path wrapper, or second validation layer
would add indirection and duplicate policy without improving the boundary.

## Limits

No GUI, QApplication, EXE, filesystem-race simulation, test-only asset,
runtime provider, or release gate was run. Delegated architecture and
independent windows returned `NO_CONCLUSION`; parent review is the only PASS
review conclusion claimed.
