# D218 / ARCH-202 parent review: workspace-search error taxonomy

## Decision

`PASS` for the bounded application taxonomy migration, accepted with explicit
runtime and release limits.

## Evidence

- `ApplicationTypeError` preserves `TypeError` compatibility while remaining
  under the shared `ApplicationError` category.
- All 39 workspace-search validation/provider-result branches now use the
  matching application-owned category.
- Exact messages, validation order, dataclass shape, provider protocol,
  directory capability, and Qt-free imports remain unchanged.
- The two path-boundary `except ValueError` clauses remain built-in catches and
  rethrow the application validation category only at the application boundary.

## Simplification assessment

`PASS`: one new built-in-compatible category and focused raise substitutions
are the smallest complete taxonomy slice; no result union, adapter wrapper, or
new exception translation service is justified.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Native filesystem traversal, worker timing, cancellation interleavings,
GUI/EXE runtime, and release evidence remain unrun.
