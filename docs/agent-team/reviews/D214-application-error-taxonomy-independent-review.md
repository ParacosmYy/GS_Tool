# D214 / ARCH-199 independent review: application error taxonomy

## Result

`PASS` for the bounded static source review by `Fermat the 6th`. A separate
`Aristotle the 6th / Luna max` review window returned `NO_CONCLUSION` after two
bounded waits and was closed; that timeout remains a review limitation, not a
contradictory finding.

## Independent evidence

- `ApplicationValidationError` remains a `ValueError` and
  `ApplicationStateError` remains a `RuntimeError`, with no custom string
  formatting that would alter existing messages.
- Document/workspace migrations preserve message literals and formatting.
- `WorkspaceService.contains()` remains compatible with the migrated
  validation category through `ValueError` inheritance.
- Application services depend on domain models, application errors, and ports;
  no infrastructure or Qt import was introduced.
- Existing presentation `Exception` boundaries preserve `str(error)` in the
  user-facing failure path.
- The source scope is limited to `errors.py`, `documents.py`, and
  `workspace.py`.

## Parent-retained checks

The parent separately checked the taxonomy inheritance, exact message
preservation, application import direction, unchanged domain conflict
ownership, existing presentation catch compatibility, compile, Ruff, format,
presentation audit, and dual-shell package identity.

## Limits

Runtime exception routing, provider/filesystem behavior, GUI/EXE interaction,
accessibility, clean-machine, cross-machine, signing, installer, support, and
release-owner evidence remain unrun. The checkout has no Git baseline, so
pre-edit textual equivalence cannot be proven beyond the bounded source scope.
