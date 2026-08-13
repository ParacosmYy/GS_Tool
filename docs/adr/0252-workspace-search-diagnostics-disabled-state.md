# ADR-0252: Workspace-search diagnostics disabled-state hierarchy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D204 / UI-107 / ARCH-190

## Context

The Find in Files diagnostics toggle uses a warning-colored capsule while
diagnostic details are available. `WorkspaceSearchDialog.set_busy()` disables
that control during an active search, but the centralized QSS previously had
no scoped disabled projection. A disabled control could therefore retain an
actionable warning appearance instead of communicating that the search owns
the current state.

## Decision

Add one scoped `:disabled` rule after the existing base, hover/focus, and
checked rules for
`QDialog#workspaceSearchDialog QToolButton#workspaceSearchDiagnosticsToggle`.
The disabled state uses `surface_2`, `border`, `border_strong`, and
`text_muted`, while retaining the control's semibold weight. The later rule
also wins when Qt reports the control as checked and disabled. No widget
behavior, signal, search state, or warning token is changed.

## Preserved invariants

- `set_busy()` still enables and disables the same controls in the same order;
  only the visual projection changes.
- The normal warning surface, hover/focus feedback, checked state, localized
  text, arrow, visibility, and diagnostics expansion behavior are unchanged.
- The selector remains owned by the centralized presentation stylesheet; no
  application-layer, search-service, or widget subclass dependency is added.
- The subdued disabled text and surface are generated from the existing theme
  tokens across the existing theme/accent matrix.

## Review and applicability

The architecture consultation (`Hypatia the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Kant the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one scoped QSS state closes the missing visual state without a new widget,
state machine, token, or behavior branch.

The applicable public first-party source is Qt's Style Sheets Reference,
which defines widget pseudo-states such as `:disabled` and selector-based
styling:
https://doc.qt.io/qt-6/stylesheet-reference.html. This is a Python 3.12 /
PyQt6 presentation-only change, not embedded C/C++, MCU, RTOS, or
manufacturer-requirement work; the mandatory embedded enterprise workflow is
therefore not applicable to this source slice. Public CloudWeGo material
remains engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Evidence and limits

- `D204-DISABLED-STATE-PROBE=PASS`
- `D204-COMPILEALL=PASS`
- `D204-RUFF=PASS`
- `D204-FORMAT=PASS`
- `D204-PRESENTATION-AUDIT=PASS`
- `D204-PACKAGE-BUILD-PS51=PASS`
- `D204-PACKAGE-BUILD-PS7=PASS`
- `D204-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native dialog/editor
rendering, accessibility tree, DPI, clean-machine, cross-machine, signing,
installer, updater, legal, support, or release-owner evidence was performed.
Native Qt selector parsing/painting and live busy-state rendering remain
open. No unit-test asset was created or run.

