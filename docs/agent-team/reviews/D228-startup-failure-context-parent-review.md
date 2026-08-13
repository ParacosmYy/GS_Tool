# D228 / ARCH-210 parent review: startup failure context record

## Decision

`PASS` for the bounded entry-boundary source change.

## Evidence

- `src/quillforge/__main__.py` keeps the existing exception, traceback,
  MessageBox, and exit-code flow and adds only fixed diagnostic context.
- `_safe_resolved_path()` catches path-resolution failures; the context tuple
  is collected behind a broad fail-open guard so it cannot mask the original
  startup exception.
- The field set contains only executable/CWD/frozen/runtime/bundle metadata;
  it does not serialize arguments, environment variables, settings, or
  document content.
- The startup-context structural probe, compileall, Ruff, formatting, and
  presentation audit pass before packaging.

## Simplification assessment

`PASS`: the helper/tuple/guard split keeps failure safety visible without
introducing a crash-reporting abstraction or moving diagnostics into Qt.

## Risks and limits

Paths may contain usernames or network roots, which is documented as an
explicit local-log privacy tradeoff. The change does not prove native startup
or repair an environment-specific failure.

