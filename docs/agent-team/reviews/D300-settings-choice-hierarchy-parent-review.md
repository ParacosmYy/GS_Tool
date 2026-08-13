# D300 parent review — Settings choice hierarchy

## Scope

Reviewed the Settings dialog object-name contract, existing generic combo-box
QSS, theme/accent swatch projection, keyboard/focus state selectors, contrast
matrix, presentation audit, source diagnostics, and package boundary.

## Findings

- PASS — the change is confined to `theme.py` and the static audit; the
  Settings dialog, values, item data, icons, signals, and snapshot contract
  are unchanged.
- PASS — `settingsTheme` and `settingsAccent` retain visible identity through
  a surface ladder, three-pixel semantic edge, and stronger typography.
- PASS — hover, focus, and open-menu states add background/border cues without
  relying on color alone.
- PASS — existing generic drop-down, item-view, disabled, and keyboard rules
  remain the shared owner for ordinary combo behavior.
- PASS — all three themes and four accents pass the normal-text contrast floor
  in the targeted QSS probe and the full presentation audit.
- PASS — source startup and file-open diagnostics remain passed; no window or
  event loop is entered.

## Simplification assessment

PASS. A small selector block in the existing centralized QSS is the smallest
complete change. A custom combo subclass, dynamic property, duplicate palette,
or settings-dialog stylesheet would add coupling and create another visual
owner without improving the contract.

## Review limits

The independent Luna/max review returned `NO_CONCLUSION` after three bounded
waits and was closed. No independent PASS is claimed. Native QSS painting,
focus metrics, accessibility, DPI, and clean-machine behavior remain
unverified.

## Applicability

Python 3.12/PyQt6 desktop presentation only. Qt stylesheet documentation is an
engineering reference; public embedded-vendor source applicability is N/A.
