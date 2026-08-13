# D192 parent review: plugin failure phase localization

## Decision

`PASS` for the bounded presentation slice, accepted with explicit runtime and
release limits.

## Review evidence

- The existing `_PLUGIN_FAILURE` message-shape regex remains the sole admission
  boundary.
- One bounded map translates only `activate`, `deactivate`, `command`, and
  `event`; unknown phases remain visible verbatim.
- Plugin IDs and error details are interpolated unchanged, preserving useful
  diagnostics.
- English identity remains an early return; no runtime or event-bus path was
  modified.

## Simplification assessment

`PASS`: a single phase map inside the existing localizer is the smallest
cohesive change. Adding phase translation to plugin runtime/application code or
introducing a new locale service would increase coupling and duplicate policy.

## Limits

The review is static. Native Qt text metrics, accessibility, DPI, GUI/EXE
startup, screenshot comparison, clean-machine behavior, and release gates were
not run under the current policy.
