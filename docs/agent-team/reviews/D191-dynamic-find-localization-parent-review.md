# D191 parent review: dynamic Find/Replace status localization

## Decision

`PASS` for the bounded presentation slice, accepted with explicit runtime and
release limits.

## Review evidence

- Two strict regular expressions cover only the existing plural count and
  Replace All limit message shapes.
- The helper delegates to the established `find.status.replaced_count` and
  `find.status.limit` catalog entries instead of embedding another language
  table.
- English remains an early-return identity path; unknown message shapes retain
  the existing safe fallback rather than becoming partially translated.
- The generic `Replaced ` prefix was removed because it could leave the
  English `matches` suffix visible after a Chinese prefix.
- No editor, worker, signal, operation policy, status level, persistence, or
  application coordinator changed.

## Simplification assessment

`PASS`: one small matcher helper and two module-level patterns are the minimum
cohesive boundary for complete dynamic-message translation. A change to the
editor operation or a new localization service would increase coupling without
improving this presentation defect.

## Limits

The review is static. Native Qt text metrics, accessibility, DPI, GUI/EXE
startup, screenshot comparison, clean-machine behavior, and release gates were
not run under the current policy.
