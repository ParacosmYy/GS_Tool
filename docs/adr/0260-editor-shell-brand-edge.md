# ADR-0260: Editor-shell brand edge

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D213 / UI-115 / ARCH-198

## Context

The central editor shell already had a token-driven gradient, neutral border,
and rounded stage. Its surrounding command rail and workspace dock exposed
clear accent edges, but the primary editing stage did not carry the same visual
anchor. That made the most important surface scan flatter than the rest of the
modernized shell.

## Decision

Add one `border-top: 2px solid {colors.accent_pink}` declaration to the
existing `QWidget#editorShell` rule in the centralized presentation stylesheet.
Keep the existing gradient, neutral border, radius, child composition, editor
palette, tab rail, and all interaction policy unchanged. The accent is resolved
from the existing theme token set, so every supported theme/accent combination
keeps the same source of truth.

## Preserved invariants

- `EditorShellSurface` remains the owner of the central widget/layout
  composition; no widget, coordinator, or state service is added.
- Document tabs, FindBar, editor content, focus, locale, motion, and file
  behavior are not changed.
- The rule is scoped to the shell container and does not recolor text or
  override child-widget state selectors.

## Review and applicability

The architecture consultation (`Mencius the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Tesla the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`. The behavior-preserving simplification assessment is `PASS`: one
existing centralized token declaration is the smallest complete visual
hierarchy correction.

The [Qt 6 Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
is the applicable public first-party source for the scoped QWidget stylesheet
projection. This is a Python 3.12 / PyQt6 presentation-only change, not
embedded C/C++, MCU, RTOS, or manufacturer-requirement work; the mandatory
embedded enterprise workflow is therefore not applicable to this source
slice. Public CloudWeGo material remains an engineering reference only; no
private ByteDance standard, certification, or compliance claim is made.

## Evidence and limits

- `D213-EDITOR-SHELL-ACCENT-SOURCE-PROBE=PASS`
- `D213-ACCENT-PINK-TOKEN-PROBE=PASS themes=3`
- `D213-COMPILEALL=PASS`
- `D213-RUFF=PASS`
- `D213-FORMAT=PASS`
- `D213-PRESENTATION-AUDIT=PASS`
- `D213-PACKAGE-BUILD-PS51=PASS`
- `D213-PACKAGE-BUILD-PS7=PASS`
- `D213-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QSS painting,
accessibility tree, DPI, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was performed. No
unit-test asset was created or run. Native QSS geometry and platform style
metrics remain open.
