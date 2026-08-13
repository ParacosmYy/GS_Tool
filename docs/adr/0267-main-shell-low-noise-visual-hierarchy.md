# ADR-0267: Main-shell low-noise visual hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D220 / UI-118

## Context

The shell had a coherent token palette but the main window, editor stage,
command rail, and document-tab rail each added gradients and nested rounded
containers. In combination, those decorations competed with the editor and
made the workspace feel heavier than the content it frames.

## Decision

Keep the existing centralized `presentation.theme` owner and flatten only
four shell selectors: `QMainWindow#mainWindow`, `QWidget#editorShell`,
`QToolBar#commandBar`, and `QTabBar#documentTabBar`. Use the existing
`surface_0`, `surface_1`, `surface_2`, `border`, `border_strong`, and accent
tokens as a clear surface ladder; remove shell gradients, reduce editor/rail
corner radii, and turn the document-tab rail into a quiet flat separator.

## Boundaries

1. Object names, actions, signals, layout, locale, fonts, motion policy,
   theme resolution, icon projection, and all widget state selectors remain
   unchanged.
2. No widget-local stylesheet, new theme token, custom title bar, animation
   rewrite, or application/domain dependency is introduced.
3. This is a visual hierarchy refinement, not a claim of native Qt rendering
   or screenshot acceptance.

## Public-source applicability and review

This is PyQt6 presentation code. Qt's public [Style Sheets
Reference](https://doc.qt.io/qt-6/stylesheet-reference.html) is the applicable
first-party source for selector/property ownership. Public CloudWeGo material
is an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

The architecture role `Halley the 6th / Luna max` returned `NO_CONCLUSION`
after two bounded waits. The independent role `Hilbert the 6th / Luna max`
returned `NO_CONCLUSION` after two bounded waits. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Verification and limits

- `D220-MAIN-SHELL-QSS-PROBE=PASS selectors=4 combos=12 gradients=0`.
- `D220-COMPILEALL=PASS`, `D220-RUFF=PASS`, `D220-FORMAT=PASS`, and
  `D220-PRESENTATION-AUDIT=PASS`.
- `D220-PACKAGE-BUILD-PS51=PASS`, `D220-PACKAGE-BUILD-PS7=PASS`, and
  `D220-PACKAGE-IDENTITY-PROBE=PASS`.

No GUI/QApplication, EXE launch, native QSS painting, screenshot,
accessibility, font fallback, DPI, clean-machine, cross-machine, signing,
installer, updater, legal, support, or release-owner evidence was run. No
unit-test asset was created or run.
