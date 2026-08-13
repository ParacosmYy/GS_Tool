# D72 parent review — plugin-dialog visual hierarchy

| Field | Value |
|---|---|
| Delivery | `D72 / UI-45` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

Centralized `theme.py` now gives Plugin Catalog and Plugin Status dialogs a
bounded outer accent/border, summary card, list panel, focus cue, and selected
row hierarchy using existing theme tokens.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. New selectors target only the
  two dialog object names and preserve existing global item/disabled states.
- **Readability/simplicity:** PASS. The slice adds one centralized selector
  group; no widget-local stylesheet, layout wrapper, or new visual state owner
  is introduced.
- **Architecture:** PASS with limits. Theme remains the only QSS owner and
  dialogs keep their existing presentation composition and policy boundaries.
- **Security:** PASS by scope. No plugin metadata, trust, process, or input
  handling is changed.
- **Performance:** PASS by scope. QSS size increases by a small static block;
  no runtime work, animation, or retained object is added.

## Simplification assessment

No further safe simplification was identified. Removing object-name scoping
would leak plugin-dialog treatment to unrelated lists, while adding per-widget
stylesheets would duplicate theme ownership. The selected-row border and
weight preserve non-color state distinction with minimal selectors.

## Authorized non-destructive validation

- `D72-THEME-HIERARCHY-CONTRAST-PROBE=PASS`.
- Targeted/full compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist hash identity — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native QSS rendering, DPI, font metrics, accessibility, callback timing,
runtime startup, clean-machine, cross-machine, and release-owner evidence
remain open. Embedded C/C++ and vendor-public-source requirements are N/A.
