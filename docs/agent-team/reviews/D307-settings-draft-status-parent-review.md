# D307 parent review — Settings unsaved-draft status

## Scope

Reviewed the Settings baseline lifecycle, snapshot equality, preview and
behavior signal paths, locale/RestoreDefaults ordering, dynamic QSS refresh,
localized accessible status text, source audit, diagnostics, and package
boundary.

## Findings

- PASS — the baseline is captured once at dialog construction and is not added
  to the domain or persistence model.
- PASS — the clean/changed decision uses the existing immutable
  `SettingsSnapshot` equality contract, covering all current settings fields.
- PASS — all preview-affecting controls update status after existing preview
  work, while behavior toggles update only the status; locale and reset reuse
  `set_locale()`.
- PASS — status text and accessible name are localized, and `draftState` gives
  the QSS a semantic state with readable clean/changed endpoints.
- PASS — format, compileall, Ruff, presentation audit, source diagnostics,
  package identity, PE header, and frozen archive checks pass.

## Simplification assessment

PASS. Existing snapshot equality provides the complete comparison without a
second dirty-state model or service. A single status helper centralizes text,
accessibility, dynamic property, and polish refresh; no further abstraction is
needed for one dialog-local projection.

## Review status and limits

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded waits. Native Qt/EXE rendering, screen
reader behavior, focus traversal, clean-machine behavior, and release gates
remain unverified. No unit tests, mocks, fixtures, or harnesses were added or
run.

## Public-source applicability

Qt dynamic property/accessibility APIs and WCAG 2.2 are public engineering
references. Embedded public-vendor applicability is N/A because this is
Python/PyQt6 desktop presentation code, not embedded C/C++ or firmware.
