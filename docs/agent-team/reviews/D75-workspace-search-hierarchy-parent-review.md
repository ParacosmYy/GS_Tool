# D75 parent review — workspace-search visual hierarchy

| Field | Value |
|---|---|
| Delivery | `D75 / UI-48` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The workspace search dialog now has scoped hierarchy for the selected root,
query, result list, diagnostic list/toggle, focus, hover, selected, checked,
and disabled states. The only dialog source change is the semantic object name
for the existing diagnostic toggle.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. Selectors target the intended
  dialog/object names and preserve existing result item data, signals,
  cancellation, and status projection.
- **Readability/simplicity:** PASS. One centralized selector block and one
  explicit object name close the visual gap without custom row widgets,
  duplicated styles, or state logic.
- **Architecture:** PASS with limits. QSS remains in `theme.py`; the dialog
  remains a presentation projection and no application/service dependency is
  introduced.
- **Security:** PASS by scope. No search query, path, diagnostic reason, or
  filesystem boundary is changed; the existing bounded diagnostic projection
  remains the data owner.
- **Performance:** PASS by scope. The change adds static QSS and no per-result
  widget, worker, timer, or additional data transformation.

## Simplification assessment

No further safe simplification was identified. Removing object-name scoping
would weaken ownership; local styles would duplicate tokens; omitting the
diagnostic identity would require a broad one-button selector. The result list
keeps native QListWidget item rendering, which is the smallest complete change.

## Authorized non-destructive validation

- `D75-SEARCH-HIERARCHY-CONTRAST-PROBE=PASS`.
- Presentation compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist identity — `PASS`.
- No `QApplication` startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native QSS rendering, layout metrics, keyboard traversal, accessibility, DPI,
fonts, runtime startup, clean-machine, cross-machine, signing, legal, and
release-owner evidence remain open. The delegated Architect and independent
reviewer did not return conclusions; no child PASS is claimed.
