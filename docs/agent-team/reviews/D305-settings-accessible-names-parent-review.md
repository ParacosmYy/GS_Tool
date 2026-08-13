# D305 parent review — Localized Settings accessible names

## Scope

Reviewed the Settings locale projection boundary, the nine value-control and
three behavior-control mappings, accessibility setter expressions, source
contract audit, package identity, and non-destructive startup/file-open
diagnostics.

## Findings

- PASS — `_refresh_accessible_names()` uses existing translated label/text
  sources, so the names follow the active locale without adding duplicate
  translation keys.
- PASS — `set_locale()` updates all relevant visible text before invoking the
  helper, and invokes the helper before the existing preview projection.
- PASS — the helper has exactly two small loops and exactly two setter
  expressions, preserving one ownership boundary for the 9+3 mapping.
- PASS — the AST-backed audit verifies method scope, exact control membership,
  setter shape, ordering, and localeChoice continuity.
- PASS — format, compileall, Ruff, presentation audit, source diagnostics,
  package identity, PE header, and frozen archive checks pass.

## Simplification assessment

PASS. A single private projection helper with two explicit loops removes twelve
duplicated setter statements while retaining an auditable mapping. New
translation keys, an accessibility service, a model field, or a second locale
pipeline would add coupling without improving this local presentation
boundary.

## Review status and limits

The architecture consultation returned `NO_CONCLUSION` after three bounded
waits. The independent review returned `PASS` for the D305 source/contract
scope and `NO_CONCLUSION` for full delivery signoff because it preceded the
new package/handoff records. Native Qt/EXE rendering, screen-reader output,
focus/accessibility behavior, DPI, clean-machine behavior, real DLL loading,
and release gates remain unverified. No unit tests, mocks, fixtures, or
harnesses were added or run.

## Public-source applicability

Qt accessibility APIs and WCAG 2.2 are public engineering references. Embedded
public-vendor applicability is N/A because this is Python/PyQt6 desktop
presentation code, not embedded C/C++ or firmware.
