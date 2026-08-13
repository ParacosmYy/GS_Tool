# D229 parent review — startup diagnostic path fail-open

## Decision

`PASS` with limits.

## Review scope

- `src/quillforge/__main__.py` only for runtime behavior.
- D228's early failure log and the existing no-Qt entry boundary.
- No change to `main`, Qt construction, `MessageBoxW`, or exit status.

## Findings

- The report path is now resolved inside a fail-open boundary, so a path or
  user-directory failure returns `None` instead of masking the original
  startup exception.
- D228's context fallback remains intact when context collection fails.
- Payload formatting and filesystem writes are protected by the same explicit
  diagnostics boundary.
- The change does not add a service, dependency, settings contract, or second
  logging owner.

## Evidence

- `D229-FAIL-OPEN-PATH-PROBE=PASS`
- `D229-CONTEXT-FALLBACK-PROBE=PASS`
- `D229-COMPILEALL=PASS`
- `D229-RUFF=PASS`
- `D229-FORMAT=PASS`
- `D229-PRESENTATION-AUDIT=PASS`

## Simplification assessment

`D229-SIMPLIFICATION-ASSESSMENT=PASS`: no smaller change preserves both the
original D228 context fallback and the new path-resolution fail-open guarantee.
The broad `Exception` boundary is intentional at this diagnostics-only edge;
its purpose is to preserve the original startup failure.

## Limits

No EXE, QApplication, native MessageBox, clean-machine, or cross-machine
launch was performed. This review does not claim native runtime success.
