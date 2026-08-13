# D227 / ARCH-209 parent review: private attribute declaration-aware audit

## Decision

`PASS` for the bounded source/static change.

## Evidence

- `scripts/audit_presentation_contracts.py` adds one helper beside the
  existing AST gates and does not change runtime application code.
- Direct private calls still require either a class method, a class-body
  declaration, or an attribute assigned by an instance method.
- The existing presentation audit passes and the structural class-declaration
  probe confirms direct `Assign`, `AnnAssign`, and `AugAssign` names are
  collected.
- Compileall, Ruff, and formatting pass before packaging.

## Simplification assessment

`PASS`: the helper is the smallest readable way to separate class-body
declarations from method traversal while keeping one known-name set. A generic
AST visitor or a broad type-checker migration would add scope without
improving this startup-typo guard.

## Risks and limits

The audit cannot infer runtime callability, dynamic `setattr`, destructuring,
or private methods supplied by external base classes. A declared non-callable
field can satisfy this heuristic; that is an explicit limitation of this
static guard, not a runtime proof. Native Qt/EXE startup remains unrun.

