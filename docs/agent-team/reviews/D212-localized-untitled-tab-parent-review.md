# D212 / UI-114 parent review: localized untitled tab title

## Decision

`PASS` for the bounded presentation change, accepted with explicit runtime and
native-rendering limits.

## Review evidence

- `_tab_title` receives the current `Locale` and selects the catalog value only
  for pathless documents; saved documents still use `state.path.name`.
- The dirty marker remains a pure projection of `state.dirty`, and all current
  call sites pass the current window locale.
- `PresentationLocalePorts` remains a frozen, slots-based Qt-free contract;
  `refresh_tab_titles` is a callback seam rather than a new state owner.
- `PresentationLocaleCoordinator.retranslate()` refreshes existing titles
  immediately after the editor shell locale and before icon refresh, keeping
  the existing locale-refresh sequence deterministic.
- `MainWindow._refresh_tab_titles()` reuses `DocumentTabSurface.set_title` and
  does not mutate document state, paths, dirty state, or open/save policy.
- The fixed English literal is absent from `main_window.py`; both locale
  catalogs contain the new key.

## Simplification assessment

`PASS`: the smallest complete fix is one catalog key, one explicit locale
parameter, and one existing-surface refresh callback. A title service, tab
model mutation, or widget-local translation branch would add coupling and
duplicate the established locale boundary.

## Limits

No GUI/QApplication, EXE launch, native tab rendering, screenshot,
accessibility, DPI, runtime open/save/recovery interaction, unit-test asset,
or release gate was run. Both delegated review windows returned
`NO_CONCLUSION`; parent review is the only claimed PASS review conclusion.
