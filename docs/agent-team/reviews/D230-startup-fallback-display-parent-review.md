# D230 parent review — startup fallback display fail-open

## Decision

`PASS` with limits.

## Review scope

- `src/quillforge/__main__.py` `_show_startup_failure` only.
- Exception summary formatting, native MessageBox fallback, stderr fallback,
  normal message compatibility, and final exit behavior.
- No Qt, application, settings, document, plugin, or packaging policy change.

## Findings

- Ordinary exceptions still produce the existing human-readable message.
- A failing exception `__str__` produces a stable generic message rather than
  replacing the original exception.
- Native MessageBox failures fall through to stderr without changing the
  final `SystemExit(1)` path.
- stderr failures are swallowed only at this diagnostics-only edge.
- No new diagnostic data source or cross-layer dependency was introduced.

## Evidence

- `D230-ERROR-STRING-FAIL-OPEN-PROBE=PASS`
- `D230-MESSAGEBOX-FAIL-OPEN-PROBE=PASS`
- `D230-COMPILEALL=PASS`
- `D230-RUFF=PASS`
- `D230-FORMAT=PASS`
- `D230-PRESENTATION-AUDIT=PASS`

## Simplification assessment

`D230-SIMPLIFICATION-ASSESSMENT=PASS`: the local guarded stages are the
smallest readable boundary that preserves ordinary message compatibility and
the original exception when all display channels fail.

## Limits

No EXE, QApplication, native MessageBox rendering, clean-machine, or
cross-machine launch was performed. This review does not claim native runtime
success.
