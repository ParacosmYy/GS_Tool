# D193 parent review: close-guard pending feedback localization

## Decision

`PASS` for the bounded presentation slice, accepted with explicit runtime and
release limits.

## Review evidence

- One catalog entry and one strict count-bearing matcher cover the exact
  message emitted by `CloseGuardFeedbackCoordinator`.
- English identity is retained; unknown shapes are not partially translated.
- The count is preserved as presentation data and the existing
  `MessageSurface`/`localize_message` route remains the owner.
- CloseGuard, TaskRunner, session-save, worker-drain, and shutdown policy are
  untouched.

## Simplification assessment

`PASS`: the catalog-backed matcher is the smallest cohesive fix. Changing the
close coordinator to know locales or introducing a separate close-message
service would increase coupling without improving the visible defect.

## Limits

The review is static. Native Qt text metrics, accessibility, DPI, GUI/EXE
startup, screenshot comparison, clean-machine behavior, and release gates were
not run under the current policy.
