# UI-60 — dialog action-rail hierarchy parent review

## Scope

Reviewed the current changes in:

- `src/quillforge/presentation/plugin_catalog_dialog.py`
- `src/quillforge/presentation/plugin_status_dialog.py`
- `src/quillforge/presentation/theme.py`

## Findings

- PASS: both wrappers use the same `dialogActionRail` identity and preserve
  existing button instances, order, connections, enablement, and locale code.
- PASS: the new wrapper has no signals, policy, plugin-service, or state
  ownership; it is a presentation-only QWidget/layout boundary.
- PASS: centralized QSS adds only a token border and button width, while
  existing primary/warning selectors retain action state/contrast ownership.
- PASS: source action-rail, token-separator, and behavior-boundary probes,
  compileall, Ruff, and format passed before packaging.

## Review result

`PASS` within the bounded source scope. Native layout/rendering, focus and
accessibility output, DPI/font metrics, and actual visual perception remain
unproven under the no-launch boundary.

## Public-source applicability

Python 3.12/PyQt6 desktop presentation code only. Embedded C/C++, MCU,
vendor-manufacturer, and firmware requirements are not applicable. Public
CloudWeGo material remains an engineering reference; no private ByteDance
standard or certification/compliance claim is made.

## Simplification

`PASS`: the shared object name and one QSS rule are smaller than introducing a
new reusable action component or duplicating per-dialog state logic.
