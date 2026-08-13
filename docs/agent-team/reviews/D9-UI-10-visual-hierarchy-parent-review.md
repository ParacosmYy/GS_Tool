# D9 UI-10 parent architecture review: visual hierarchy and highlight states

- **Status:** PASS WITH LIMITS
- **Slice:** D9 / UI-10 visual hierarchy and state highlighting
- **Parent reviewer:** `architect` (current checkout sole writer)
- **Independent architect:** `Raman` / `019fe7ae-b3c8-7340-971b-0fa8254b20ef` (read-only)
- **Date:** 2026-08-10

## Review scope

The user reported that many controls lacked clear highlighting and that the
modern/cute shell still felt visually flat. The review covered the existing
`ThemeColors` registry, Sakura/legacy theme projection, and presentation
object names. The implementation is intentionally limited to
`src/quillforge/presentation/theme.py`.

## Architecture findings

The existing visual token source is coherent, but the previous QSS did not
fully distinguish checked/active, disabled/readonly, menu, combo-popup,
primary-button, and list focus states. The accepted change adds one bounded
`on_accent` token for foreground contrast on saturated action states, then
reuses the existing selection, hover, pressed, accent, surface, border, muted,
warning, and error tokens for the remaining state selectors.

No domain/application/settings/i18n/MainWindow/WorkspacePanel ownership was
changed. No business state was inferred from styling, and no new widget or
animation framework was introduced. The QScintilla editor projection remains
separate from the application shell.

## Independent review result

Raman returned **PASS WITH LIMITS** and confirmed:

- the change should remain in `presentation/theme.py`;
- the full hierarchy requires explicit QSS selectors in addition to tokens;
- all Ink, Paper, Sakura, and accent combinations must keep complete tokens;
- QSS specificity/order, list selector breadth, and runtime contrast remain
  risks requiring static and later authorized visual validation.

The parent accepted those limits and integrated only the bounded theme slice.

## Simplification assessment

The change is behavior-preserving and simpler than adding per-widget state
logic: one token registry and one stylesheet continue to own visual policy;
existing selectors and object names are reused; no duplicated theme state,
cross-layer dependency, custom control, or test-only asset was added.

## Public-source applicability and assurance gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++, MCU,
firmware, BSP/HAL, RTOS, ISR/DMA, driver, bootloader, Flash/NVM, power, or
motor-control code. The mandatory embedded enterprise and embedded
code-review/simplifier skills were invoked for the assurance gate and recorded
as **not applicable**; no public first-party embedded vendor source or
manufacturer requirement applies, and no MISRA/ISO/certification claim is
made.

## Authorized non-destructive validation

- `uv run python -m compileall -q src/quillforge` — PASS.
- `uv run ruff check src/quillforge` — PASS.
- `uv run ruff format --check src/quillforge` — PASS.
- Theme contract smoke — PASS for 3 themes × 4 accents and stylesheet
  generation without launching Qt.
- QApplication/EXE launch, screenshot, installed-font/DPI rendering, native
  style metrics, and interactive state verification — intentionally unrun
  under the active no-launch policy.

## Disposition

`accepted-with-limits`: visual state hierarchy is centralized and statically
verified; runtime visual acceptance, formal contrast measurement, and
cross-machine rendering remain open for an authorized reviewer.
