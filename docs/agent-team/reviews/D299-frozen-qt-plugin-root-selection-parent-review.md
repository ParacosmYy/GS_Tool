# D299 parent review — frozen Qt plugin root selection

## Scope

Reviewed the frozen PyQt6 plugin-root selector, D296 environment binding,
D297 runtime preflight, startup diagnostic path, static presentation contract,
source diagnostics, selector simulation, and rebuilt archive.

## Findings

- PASS — the selector prefers a candidate containing
  `platforms/qwindows.dll`, while retaining Qt6-first ordering when both roots
  are usable.
- PASS — `_configure_frozen_qt_plugins()` and `_startup_qt_plugin_path()` use
  the same selector, preventing configuration/diagnostic split-brain.
- PASS — when no candidate contains `qwindows.dll`, the deterministic existing
  directory fallback preserves the existing actionable preflight path.
- PASS — the targeted audit guards the selector and the platform-plugin
  predicate.
- PASS — source startup and file-open diagnostics, project checks, package
  identity, PE headers, and archive contents remain valid.
- PASS — no normal composition, theme, document, plugin, or user-data policy
  changed.
- PASS — the first independent review's `REVISE` finding was addressed by
  strengthening the audit with AST-scoped order, file-probe/fallback, reuse,
  and import-order checks; the package was rebuilt afterward.

## Simplification assessment

PASS. One small selector reused at both existing infrastructure boundaries is
the smallest complete fix. Duplicating qwindows selection in configuration and
diagnostic code would increase drift and coupling.

## Review limits

The first independent review returned `REVISE` because the original audit was
too substring-based and its package observation predated the rebuild. The
contract was revised and the candidate rebuilt. A final independent Luna/max
review then returned `PASS` for the current package identity and static
contract. A physical split-layout bundle, native EXE startup, and clean-machine
behavior remain unverified.

## Applicability

Python 3.12/PyQt6/PyInstaller desktop code only. Public embedded-vendor source
applicability is N/A.
