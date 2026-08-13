# ADR-0345: Theme-aware Tooltip surface

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D309 / UI-128 / ARCH-279

## Context

Workspace actions now expose localized tooltips, but the shell had no shared
Tooltip QSS. Qt/Fusion or Windows defaults could therefore make the new hints
look detached from the selected theme, reduce hierarchy, or weaken readability
on bright/dark surfaces.

## Decision

Add one global `QToolTip` rule inside the existing `_stylesheet()` renderer.
Use `surface_3` for the surface, `text_primary` for the message, existing
border tokens for containment, and a readable `accent_alt` edge that falls
back to `text_primary` when the preferred edge misses the 3:1 non-text floor.
The existing UI font family, size, and style remain inherited from the shared
typography rule; the Tooltip only applies a small relative size and weight.

No Tooltip owner, locale service, widget behavior, or business flow changes.
All tooltips—including D308's folder/file hints—receive the same centralized
visual treatment.

## Evidence

- `D309-QSS-MATRIX=PASS themes=3 accents=4 tooltip_states=base minimum_text=11.16 minimum_edge=3.65`
- `D309-SOURCE-PROBE=PASS tooltip_selector=1 token_bound=1 fallback_edge=1 centralized_stylesheet=1 behavior_unchanged=1`
- `D309-COMPILEALL=PASS`
- `D309-RUFF=PASS`
- `D309-FORMAT=PASS`
- `D309-AUDIT=PASS`
- The rebuilt root/dist candidate has SHA-256
  `909A47FF0D952DB7FC8F91279982BDB4B67E744756BD2AE7B91B092EE944BFC5` and
  38,598,697 bytes.

## Review and applicability

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded Luna/max waits. Parent review and
behavior-preserving simplification assessment are `PASS`.

Qt QSS, QWidget Tooltip behavior, palette, and WCAG 2.2 are public engineering
references for this Python 3.12/PyQt6 desktop UI. Embedded vendor-source
applicability is N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code
changed. No manufacturer, MISRA, ISO 26262, ASPICE, certification, or private
corporate-standard claim is made.

## Limits

Native EXE/Qt launch, actual Tooltip painting, screen-reader output, DPI,
clean-machine behavior, real DLL loading, signing, installer, updater, and
release-owner gates remain unrun under the active non-destructive launch
policy. Unit tests, mocks, fixtures, and test harnesses were not created or
run.
