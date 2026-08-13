# ADR-0257: Default document filter coverage

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D210 / UI-112 / ARCH-195

## Context

The Workspace and command flows already reach the same native file-picker and
asynchronous document-open boundary. The default localized file filter listed
only five common extensions even though `FileDocumentStore` can load ordinary
text files by encoding detection. Users could therefore see a file as missing
from the default view unless they manually selected the existing all-files
fallback.

## Decision

Expand the existing `dialog.text_filter` catalog entry in English and
Simplified Chinese to include 18 common text/source extensions: `txt`, `md`,
`py`, `json`, `csv`, `yaml`, `yml`, `toml`, `js`, `ts`, `html`, `css`, `xml`,
`ini`, `log`, `sql`, `sh`, and `ps1`. Keep one `;;` separator and the existing
`All files (*.*)` fallback. The same catalog entry continues to serve open and
save dialogs, so no second filter policy is introduced.

## Preserved invariants

- `FileDialogSurface.choose_document()` still calls
  `QFileDialog.getOpenFileName()` and returns one optional `Path`.
- `choose_save_path()` still calls `getSaveFileName()` with the same filter,
  and `choose_workspace()` remains a directory-only flow.
- `DocumentPickerAdmissionCoordinator`, `DocumentOpenCoordinator`,
  `DocumentService`, encoding detection, persistence, async dispatch, and
  error policy remain unchanged.
- The filter remains user-selectable and never prevents selecting an
  unlisted file because `All files (*.*)` remains present.

## Review and applicability

The architecture consultation (`Beauvoir the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Noether the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one existing localized catalog value is the smallest complete change without
duplicating dialog policy or widening the document service contract.

Qt's public `QFileDialog` documentation is the applicable first-party source;
it documents name-filter groups separated by `;;`, the static open/save
convenience functions, and the all-files fallback pattern:
https://doc.qt.io/qt-6/qfiledialog.html. This is a Python 3.12 / PyQt6
presentation/localization change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material is an
engineering reference only; no private ByteDance standard, certification, or
compliance claim is made.

## Evidence and limits

- `D210-DIALOG-FILTER-PROBE=PASS locales=2 extensions=18 all-files-fallback=present`
- `D210-COMPILEALL=PASS`
- `D210-RUFF=PASS`
- `D210-FORMAT=PASS`
- `D210-PRESENTATION-AUDIT=PASS`
- `D210-PACKAGE-BUILD-PS51=PASS`
- `D210-PACKAGE-BUILD-PS7=PASS`
- `D210-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native file dialog, live file
open/save, accessibility tree, DPI, clean-machine, cross-machine, signing,
installer, updater, legal, support, or release-owner evidence was performed.
No unit-test asset was created or run. Native filter rendering and platform
case-sensitivity remain open.

