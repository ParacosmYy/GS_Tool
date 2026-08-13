# D197 parent review: packaged measurement PowerShell compatibility

## Decision

`PASS` for the bounded tooling slice, accepted with explicit runtime and
external release limits.

## Review evidence

- The diff is confined to the two JSON reads and one local compatibility
  adapter in `scripts/measure_packaged.ps1`.
- Windows PowerShell 5.1 avoids the unsupported `DateKind` parameter while
  PowerShell 7 retains the previous string-date behavior.
- Explicit UTF-8 decoding protects Chinese report content without changing
  artifact binding, capture validation, process lifecycle, cleanup, or report
  fields.
- No EXE was started, and no packaged performance result was claimed.

## Simplification assessment

`PASS`: the adapter and two encoding flags are the smallest cohesive fix;
duplicating the measurement script or adding a shared runtime module would
increase coupling and introduce unnecessary execution paths.

## Limits

The review is static plus package refresh. Native/runtime capture, clean
machine, cross-machine, and release-owner evidence remain unrun.
