# ADR-0253: Find/Replace navigation disabled-state hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D205 / UI-108 / ARCH-191

## Context

During a cooperative Replace All operation, `FindBar.set_operation_active()`
disables the previous/next navigation buttons so the current editor operation
owns the query controls. Their scoped normal and hover/focus QSS rules were
more specific than the global `QPushButton:disabled` rule, so the disabled
buttons lacked an explicit subdued projection and could still read as
actionable.

## Decision

Add one scoped disabled-state rule for
`QWidget#findBar QPushButton#findPrevious` and
`QWidget#findBar QPushButton#findNext`, ordered after their normal and
hover/focus rules. The disabled state uses the existing `surface_2`, `border`,
and `text_muted` tokens. Existing dimensions, icons, labels, operation
signals, and the global semantic button roles remain unchanged.

## Preserved invariants

- `FindBar.set_operation_active()` still disables the same query, case,
  navigation, and replacement controls and shows/enables cancel in the same
  order.
- Normal, hover, focus, pressed, warning, primary, quiet, locale, and
  keyboard behavior remain unchanged when the controls are available.
- The rule remains in centralized presentation QSS; no application-layer,
  editor, operation coordinator, or widget subclass dependency is introduced.
- Existing theme/accent token resolution remains the only color source; the
  disabled text remains readable against `surface_2` in all 12 projections.

## Review and applicability

The architecture consultation (`Gauss the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Pauli the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one grouped scoped rule is the smallest complete correction for two controls
with identical semantics.

The applicable public first-party source is Qt's Style Sheets Reference,
which documents selector pseudo-states such as `:disabled`:
https://doc.qt.io/qt-6/stylesheet-reference.html. This is a Python 3.12 /
PyQt6 presentation-only change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material
remains engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Evidence and limits

- `D205-DISABLED-STATE-SOURCE-PROBE=PASS`
- `D205-FINDBAR-BEHAVIOR-PROBE=PASS`
- `D205-QSS-DISABLED-CONTRAST-PROBE=PASS combinations=12 min=5.14`
- `D205-COMPILEALL=PASS`
- `D205-RUFF=PASS`
- `D205-FORMAT=PASS`
- `D205-PRESENTATION-AUDIT=PASS`
- `D205-PACKAGE-BUILD-PS51=PASS`
- `D205-PACKAGE-BUILD-PS7=PASS`
- `D205-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native QSS painting,
accessibility tree, DPI, live editor operation, clean-machine, cross-machine,
signing, installer, updater, legal, support, or release-owner evidence was
performed. Native Qt selector parsing/painting and operation interleavings
remain open. No unit-test asset was created or run.

