# D301 parent review — Semantic Settings choice role

## Scope

Reviewed the D301 source diff, SettingsDialog property declaration order,
existing item/data/icon/signal/snapshot behavior, centralized theme QSS state
scope, disabled-state compatibility, the presentation audit, contrast matrix,
source diagnostics, and package boundary.

## Findings

- PASS — `settingsTheme` and `settingsAccent` retain their object names and
  receive `settingsRole="identityChoice"` before item population.
- PASS — item order, `UserRole` data, swatch refresh, locale refresh,
  `currentIndexChanged` preview wiring, `settings_snapshot()`, persistence,
  keyboard behavior, and application ownership are unchanged.
- PASS — normal, hover, focus, and open-menu hierarchy is now expressed once
  through the semantic role; the existing ID-based disabled rule remains
  available for compatibility.
- PASS — the old dedicated normal/hover/focus/open ID selector block is gone;
  no duplicate visual owner or second QSS engine was introduced.
- PASS — the 3-theme × 4-accent matrix has all four semantic states and a
  minimum measured text contrast of 8.64:1 against the checked surfaces.
- PASS — compileall, Ruff, format, presentation audit, source startup/file-open
  diagnostics, package identity, and static PE/archive checks are in scope for
  final delivery.

## Architecture follow-up

The initial Luna/max architecture consultation returned `NO_CONCLUSION` after
three bounded waits. A follow-up architecture review identified that the D300
audit still targeted removed ID selectors and that D300/D301 visual assertions
were duplicated (`REVISE`). The parent applied the smallest correction: one
canonical state-table helper plus declaration-order and legacy-selector checks.
The final audit passes.

## Simplification assessment

PASS. Consolidating the visual state fragments into one small data contract
removes duplicate ownership and reduces future extension cost. A generic QSS
parser, custom combo subclass, new theme abstraction, or second settings style
owner would add coupling without changing behavior.

## Limits

Independent review returned `NO_CONCLUSION` after three bounded waits; no
independent PASS is claimed. Native Qt/EXE rendering, focus/accessibility,
DPI/font metrics, clean-machine behavior, and real DLL loading remain
unverified. Unit tests, mocks, fixtures, and test harnesses were not created
or run under project policy.

## Public-source applicability

Qt 6.11.1 QObject and Qt Style Sheets documentation is an engineering
reference. Public embedded-vendor source applicability is N/A; this is Python
3.12/PyQt6 desktop presentation code and no embedded compliance claim is made.
