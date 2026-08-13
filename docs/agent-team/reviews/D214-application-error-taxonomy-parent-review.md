# D214 / ARCH-199 parent review: application error taxonomy

## Decision

`PASS` for the bounded Phase 3 application-layer change, accepted with
explicit runtime and release limits.

## Review evidence

- `application.errors` is dependency-free and contains only stable taxonomy
  definitions.
- `ApplicationValidationError` remains a `ValueError`, and
  `ApplicationStateError` remains a `RuntimeError`; existing presentation
  catch sites remain source-compatible.
- `DocumentService` changes only the missing-target invariant; its exact
  message, persistence port, revision handling, and returned state are intact.
- `WorkspaceService` changes only the application-owned validation/state
  categories; path resolution, containment, provider calls, root activation,
  and messages remain intact.
- Domain `DocumentConflictError` and infrastructure adapter errors remain in
  their existing ownership layers.

## Simplification assessment

`PASS`: a single error module plus two focused imports is smaller and clearer
than wrapping every adapter failure, adding a service-wide result union, or
moving domain conflicts into application policy.

## Limits

No GUI/QApplication, EXE launch, runtime filesystem interaction, unit-test
asset, or release gate was run. The architecture window returned
`NO_CONCLUSION`; independent static review by Fermat returned `PASS`, while a
second independent window returned `NO_CONCLUSION` after bounded waits.
