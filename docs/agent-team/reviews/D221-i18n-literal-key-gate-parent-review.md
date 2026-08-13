# D221 / ARCH-204 parent review: i18n literal-key static gate

## Decision

`PASS` for the bounded AST/static gate, accepted with explicit runtime and
dynamic-key limits.

## Evidence

- The existing presentation contract audit remains the only changed tool
  boundary.
- Literal `tr()` keys are compared with the canonical `_ENGLISH` catalog.
- Dynamic keys are intentionally not rejected, and runtime fallback is
  unchanged.
- The 3-theme/accent and UI runtime layers are not touched.

## Simplification assessment

`PASS`: extending the existing audit is smaller and clearer than adding a
second catalog loader, runtime instrumentation, or a new translation service.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Dynamic-key validity, native rendering, runtime locale refresh, and release
evidence remain open.
