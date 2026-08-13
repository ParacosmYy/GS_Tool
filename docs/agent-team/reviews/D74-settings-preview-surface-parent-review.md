# D74 parent review — settings preview surface boundary

| Field | Value |
|---|---|
| Delivery | `D74 / UI-47` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

`SettingsPreviewSurface` now owns the D73 preview object tree, layout,
localized copy, accessibility name, and token/style projection. `SettingsDialog`
retains the controls, snapshot construction, Save/Cancel, and application
policy, passing one current selection through `project(...)`.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. The surface is parented by the
  dialog, receives current control values, and has no persistence or global
  theme call. Locale changes still flow through the dialog's existing
  retranslation path.
- **Readability/simplicity:** PASS. The dialog loses the preview widget-tree
  details and retains one named projection call; the surface has one focused
  responsibility and no speculative model or signal layer.
- **Architecture:** PASS with limits. Dependencies remain
  presentation → domain value types and presentation theme/i18n helpers;
  application/domain modules do not import the surface. Theme/QSS ownership
  remains canonical.
- **Security:** PASS by scope. The surface receives bounded combo values and
  display labels; no external HTML, path, process, plugin, or file input is
  added.
- **Performance:** PASS by scope. The same small local projection is retained;
  extraction adds no worker, timer, image, or repeated global stylesheet.

## Simplification assessment

No further safe simplification was identified. A view-model would add a second
state layer; merging the surface back would restore mixed responsibilities;
moving token resolution out of `theme.py` would duplicate visual ownership.
The `project(...)` API is intentionally one call so locale and selection cannot
drift across separate refresh methods.

## Authorized non-destructive validation

- `D74-PRESENTATION-BOUNDARY-PROBE=PASS`.
- Presentation compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist identity — `PASS`.
- No `QApplication` startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native Qt parent/layout/rendering, modal interaction, installed-font fallback,
keyboard traversal, accessibility, DPI, runtime startup, clean-machine,
cross-machine, signing, legal, and release-owner evidence remain open. The
delegated Architect and independent reviewer did not return conclusions; no
child PASS is claimed.
