# ADR-0259: Localized untitled tab titles

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D212 / UI-114 / ARCH-197

## Context

Pathless documents used a fixed English `Untitled` tab title. The existing
locale refresh updated the editor shell and tab icons, but did not reproject
titles already present in the tab surface. That left a newly selected Chinese
locale with a mixed-language tab label while saved documents correctly used
their file names.

## Decision

Treat the pathless title as presentation text supplied by the existing i18n
catalog. Extend `_tab_title` with an explicit `Locale` input, use the
localized `document.untitled` key only when `DocumentState.path` is absent,
and preserve the existing dirty-marker and saved-file-basename behavior. Add
one `refresh_tab_titles` callback to the Qt-free
`PresentationLocalePorts` boundary and call it after the editor shell locale
is set, so existing tabs are reprojected without changing document state or
file paths.

## Preserved invariants

- Saved documents continue to display `Path.name`; locale changes never
  rename or rewrite a file-backed document.
- Dirty state continues to be represented by the existing leading `*`.
- New/open/save/recovery/close flows keep their existing document and message
  boundaries; only the presentation title source changes.
- Locale orchestration remains a frozen, slots-based Qt-free ports contract;
  `MainWindow` wires the existing `DocumentTabSurface.set_title` seam rather
  than adding a second tab registry or title service.

## Review and applicability

The architecture consultation (`Faraday the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Kepler the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`. The behavior-preserving simplification assessment is `PASS`: one
i18n key, one explicit locale argument, and one existing tab-surface refresh
seam are the smallest complete fix.

The [Qt 6 QTabWidget documentation](https://doc.qt.io/qt-6/qtabwidget.html)
is the applicable public first-party source for the existing tab-title
projection surface. This is a Python 3.12 / PyQt6 presentation change, not
embedded C/C++, MCU, RTOS, or manufacturer-requirement work; the mandatory
embedded enterprise workflow is therefore not applicable to this source
slice. Public CloudWeGo material remains an engineering reference only; no
private ByteDance standard, certification, or compliance claim is made.

## Evidence and limits

- `D212-LOCALIZED-TAB-TITLE-SOURCE-PROBE=PASS`
- `D212-LOCALE-REFRESH-ORDER-PROBE=PASS`
- `D212-FIXED-UNTITLED-REGRESSION-PROBE=PASS`
- `D212-I18N-KEY-PROBE=PASS locales=2`
- `D212-COMPILEALL=PASS`
- `D212-RUFF=PASS`
- `D212-FORMAT=PASS`
- `D212-PRESENTATION-AUDIT=PASS`
- `D212-PACKAGE-BUILD-PS51=PASS`
- `D212-PACKAGE-BUILD-PS7=PASS`
- `D212-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native tab painting,
accessibility tree, DPI, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was performed. No
unit-test asset was created or run. Native widget metrics and runtime locale
refresh remain open.
