# ADR-0238: Command Palette empty-state boundary

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D190 / UI-100 / ARCH-176

## Context

The Command Palette already filtered the stable command snapshot and projected
each result as a selectable `QListWidgetItem`. When the registry was empty or a
query matched nothing, the list remained visually blank. That left keyboard
users without a clear explanation while the existing hint only described
Enter/Esc actions.

## Decision

Keep `CommandPaletteDialog` as the presentation owner and compose the existing
list with one localized `QLabel#commandPaletteEmpty` inside a single
`QStackedLayout`. `_render` continues to clear and rebuild the same list in the
same command order; it selects the empty message only when no rows exist and
chooses initial versus filtered-no-match copy from the already-normalized
query. The stage shows the list whenever rows exist.

## Preserved invariants

- The command iterable is captured once and command metadata/ordering are
  unchanged.
- `_COMMAND_ROLE`, stable command IDs, current-row selection, Enter handling,
  item activation, and `selected_command_id` projection are unchanged.
- Esc and modal acceptance remain owned by `QDialog`/the existing surface.
- Locale ownership remains `CommandPaletteSurface`; each dialog is created with
  the current locale and no new locale service is introduced.
- QSS remains centralized in `presentation.theme`; the empty state adds no
  second styling system or execution policy.

## Review and applicability

The architecture consultation (`Poincare the 6th / Luna max`) and independent
review (`Laplace the 6th / Luna max`) both timed out within their bounded
windows; both are recorded as `NO_CONCLUSION`. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS`.

This is a Python 3.12/PyQt6 presentation change. Embedded C/C++, MCU, RTOS,
and manufacturer requirements are not applicable. Public CloudWeGo material
is an engineering reference only; this ADR makes no private ByteDance
standard, certification, or compliance claim.

## Evidence and limits

- `D190-EMPTY-STATE-CONTRACT-PROBE=PASS`
- `D190-LOCALE-AND-QSS-PROBE=PASS`
- `D190-KEYBOARD-EXECUTION-PRESERVATION-PROBE=PASS`
- `D190-COMPILE-RUFF-FORMAT=PASS`
- `D190-PRESENTATION-AUDIT=PASS`
- `D190-PACKAGE-BUILD=PASS`
- `D190-PACKAGE-IDENTITY-PROBE=PASS`

Native Qt stack/list metrics, accessibility-tree output, DPI, screenshot
review, GUI/EXE startup, clean-machine, cross-machine, legal, signing,
installer, updater, support, and release-owner evidence remain open. Release
verification remains `no-go` under the current no-launch and external-gate
policy.

