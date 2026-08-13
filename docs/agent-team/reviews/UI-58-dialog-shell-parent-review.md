# UI-58 / dialog-shell edge hierarchy — parent review

## Scope

Reviewed the UI-58 source change in the current local checkout:

- `src/quillforge/presentation/theme.py`
- `src/quillforge/presentation/settings_dialog.py`
- `src/quillforge/presentation/command_palette.py`
- `docs/adr/0146-dialog-shell-edge-hierarchy.md`

## Findings

- PASS: `QDialog#settingsDialog` and `QDialog#commandPalette` are stable
  existing object-name identities; no constructor or signal code changed.
- PASS: the new frame and top accents are centralized in `_stylesheet(...)`
  and consume `surface_1`, `border_strong`, `accent_alt`, and `accent_pink`.
- PASS: selectors are object-scoped; generic `QDialog`, plugin, and
  workspace-search contracts remain present and child selectors retain their
  existing ownership.
- PASS: no hard-coded endpoint color, new state owner, wrapper widget, or
  dialog inheritance layer was introduced.
- PASS: source probe, token-endpoint probe, behavior-boundary probe,
  compileall, Ruff check, and format check passed before packaging.

## Review result

`PASS` within the bounded source scope. Native QSS client-frame rendering,
actual dialog geometry, DPI/font behavior, accessibility, and runtime visual
perception remain unproven under the active no-launch boundary.

## Public-source applicability

Python 3.12/PyQt6 desktop presentation code only. Embedded C/C++, MCU,
vendor-manufacturer, and firmware requirements are not applicable. Public
CloudWeGo material remains an engineering reference; no private ByteDance
standard or certification/compliance claim is made.

## Simplification

`PASS`: the two existing object-scoped selector groups are the smallest clear
change. A shared dialog base class or extra wrapper would add coupling without
improving the visual contract.
