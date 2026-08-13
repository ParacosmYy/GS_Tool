# D219 / ARCH-203 parent review: remaining application error taxonomy

## Decision

`PASS` for the bounded application exception-contract migration, accepted
with explicit runtime and release limits.

## Evidence

- Nine Qt-free application modules no longer raise generic
  `ValueError`/`RuntimeError` directly.
- `ApplicationValidationError` and `ApplicationStateError` preserve the
  original built-in catch categories and exact messages.
- Dedicated recovery-channel protocol exceptions remain unchanged.
- Domain, infrastructure, and presentation layers were not pulled into the
  application taxonomy.
- Dataclass shapes, protocol boundaries, validation order, and policy behavior
  remain unchanged by the substitutions.

## Simplification assessment

`PASS`: the smallest complete change is to reuse the existing three
application categories and replace only application-owned raises. No wrapper,
result-union redesign, or cross-layer exception service is justified.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
Native event-thread behavior, plugin-host exchange, filesystem behavior,
GUI/EXE runtime, and release evidence remain unrun.
