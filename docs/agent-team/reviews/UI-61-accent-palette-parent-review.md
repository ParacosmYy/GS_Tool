# UI-61 — accent palette swatch hierarchy parent review

## Scope

Reviewed the current changes in:

- `src/quillforge/presentation/icons.py`
- `src/quillforge/presentation/settings_dialog.py`

## Findings

- PASS: `color_swatch_icon` is a stateless presentation renderer and reuses
  the existing Qt vector-painting module without adding a resource or domain
  dependency.
- PASS: SettingsDialog calls the public `theme_colors` resolver and assigns
  icons only after Theme/Accent item data is populated; theme/accent changes
  refresh the visual projection without changing current data or signals.
- PASS: localized item text, `settings_snapshot`, preview projection, Save /
  Cancel wiring, and `ThemeId`/`AccentId` values remain unchanged.
- PASS: all swatch endpoints come from canonical theme tokens; Amber on
  Paper/Sakura/Ink follows the same token path as the rest of the shell.

## Review result

`PASS` within the bounded source scope. Native QComboBox rendering, focus,
screen-reader output, DPI/font metrics, and actual human visual perception
remain unproven under the no-launch boundary.

## Public-source applicability

Python 3.12/PyQt6 desktop presentation code only. Embedded C/C++, MCU,
vendor-manufacturer, and firmware requirements are not applicable. Public
CloudWeGo material remains an engineering reference; no private ByteDance
standard or certification/compliance claim is made.

## Simplification

`PASS`: one generic swatch renderer plus one refresh boundary is smaller than
introducing a custom combo delegate, a reusable settings widget, or a second
palette model.
