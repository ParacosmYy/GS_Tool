# D304 parent review — Locale choice presentation role

## Scope

Reviewed the Settings language selector, dynamic-property order, locale item
data and current-value path, immediate locale refresh, centralized QSS state
precedence, edge contrast resolution, audit coverage, source diagnostics, and
packaged artifact boundary.

## Findings

- PASS — `settingsLanguage` keeps its object name, English/Chinese item data,
  and current-index setup; `localeChoice` is declared before population.
- PASS — `currentIndexChanged` still routes through `_on_language_changed()` to
  `set_locale()`, and `SettingsSnapshot` still reads `currentData()`.
- PASS — one semantic QSS contract covers normal, hover, focus, open, and
  disabled states; the old concrete language selector group is removed.
- PASS — the pure edge resolver keeps the locale edge at or above 3:1 across
  surface, hover, and pressed backgrounds; text states pass 4.5:1 in the
  3-theme × 4-accent matrix.
- PASS — format, compileall, Ruff, presentation audit, source diagnostics,
  package identity, PE header, and frozen archive checks pass.

## Simplification assessment

PASS. The semantic role removes the last language-specific active selector
group and reuses the existing token-resolution boundary. A locale-specific
widget, tone abstraction, settings-model change, or QSS parser would add
coupling without improving the current behavior.

## Review status and limits

The bounded architecture consultation and independent review each returned
`NO_CONCLUSION` after three waits; this record makes no independent PASS claim.
Native Qt/EXE rendering, focus/accessibility, DPI, clean-machine behavior,
real DLL loading, and release gates remain unverified. No unit tests, mocks,
fixtures, or harnesses were added or run.

## Public-source applicability

Qt Style Sheets and WCAG 2.2 SC 1.4.11 are public engineering references.
Embedded public-vendor applicability is N/A because this is Python/PyQt6
desktop presentation code, not embedded C/C++ or firmware.
