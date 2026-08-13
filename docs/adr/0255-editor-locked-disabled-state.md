# ADR-0255: Editor locked disabled-state hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D208 / UI-110 / ARCH-193

## Context

Replace All locks the active editor through the existing
`EditorWidget.set_operation_locked()` adapter while the cooperative operation
runs. The editor's centralized QSS had normal and focus rules but no disabled
projection, so the main canvas could continue to look fully editable even
while input was intentionally blocked.

## Decision

Add one scoped `QsciScintilla#editor:disabled` rule after the normal and focus
rules. The locked canvas uses `surface_1`, a stronger neutral border, and the
existing `pressed` selection background. It deliberately does not override
text, lexer, or syntax colors, keeping the document readable while the canvas
communicates the lock.

## Preserved invariants

- Replace All still locks and unlocks the same editor through the existing
  typed coordinator and adapter port; no operation or lifecycle code changes.
- Normal, focus, selection, caret, lexer, syntax, font, wrapping, and editor
  settings remain unchanged when the editor is available.
- No broad disabled text color is introduced, so QScintilla syntax colors are
  not muted by the state cue.
- Theme/accent resolution remains the only color source; primary text remains
  readable against the candidate `surface_1` canvas in all 12 projections.

## Review and applicability

The architecture consultation (`Feynman the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Lovelace the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one scoped canvas rule communicates lock ownership without a new editor state,
lexer policy, or coordinator.

The applicable public first-party source is Qt's Style Sheets Reference,
which documents selector pseudo-states such as `:disabled`:
https://doc.qt.io/qt-6/stylesheet-reference.html. This is a Python 3.12 /
PyQt6/QScintilla presentation-only change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material
remains engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Evidence and limits

- `D208-DISABLED-STATE-SOURCE-PROBE=PASS`
- `D208-QSS-CANVAS-CONTRAST-PROBE=PASS combinations=12 min=12.87`
- `D208-COMPILEALL=PASS`
- `D208-RUFF=PASS`
- `D208-FORMAT=PASS`
- `D208-PRESENTATION-AUDIT=PASS`
- `D208-PACKAGE-BUILD-PS51=PASS`
- `D208-PACKAGE-BUILD-PS7=PASS`
- `D208-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QScintilla painting,
accessibility tree, DPI, live Replace All operation, clean-machine,
cross-machine, signing, installer, updater, legal, support, or release-owner
evidence was performed. Native QSS/QScintilla rendering and operation
interleavings remain open. No unit-test asset was created or run.

