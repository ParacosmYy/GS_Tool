# D226 / ARCH-208 parent review: private self-call contract audit

## Decision

`PASS` for the bounded source/static change.

## Evidence

- `scripts/audit_presentation_contracts.py` adds one named AST audit helper and
  composes it into the existing presentation contract audit.
- Direct `self._private()` calls are checked against class-defined methods and
  assigned attributes, preserving injected callable providers.
- The current presentation tree reports zero violations, including the fixed
  `CommandSurface._locale` method and its nine direct call sites.
- Compileall, Ruff, formatting, packaging, and archive/identity probes pass.

## Simplification assessment

`PASS`: the smallest clear implementation is one helper beside the existing
AST gates. Extracting a generic visitor or introducing a type-checker contract
would increase concepts without improving this narrow failure guard.

## Limits

The delegated architecture and independent review windows returned
`NO_CONCLUSION`. The rule does not model dynamic `getattr`, external base-class
private methods, or runtime callability; native Qt/EXE startup remains
unrun under the project boundary.

