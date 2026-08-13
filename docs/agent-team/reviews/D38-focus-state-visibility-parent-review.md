# D38 parent review — focus-state visibility

- **Delivery:** D38 / UI-24 / ARCH-28
- **Date:** 2026-08-10
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D38 changes only the centralized QSS returned by
`presentation.theme._stylesheet`. Focused command-rail/tool buttons now gain
the existing hover surface and primary foreground; focused document tabs gain
the same surface/foreground plus an accent-alt boundary; focused checkboxes
gain a restrained row cue and an accent-alt indicator border. Existing
selected, pressed, checked, and disabled selectors remain in place.

No widget signal, command callback, document state, settings state, locale
route, theme preference, motion policy, or application dependency boundary
moved. The change is presentation-only and remains removable as one QSS slice.

The required architecture consultation was attempted with Descartes the 2nd /
Luna max using a read-only theme-boundary question. Two bounded waits returned
no conclusion and the agent was closed; no architecture PASS is claimed. An
independent read-only review was attempted with Einstein the 2nd / Luna max;
two bounded waits also returned no conclusion and it was closed. No child
review PASS is claimed.

## Simplification assessment

Reusing `surface_hover`, `text_primary`, and `accent_alt` avoids adding a new
focus token or a parallel focus-state model. Keeping the selectors centralized
is the smallest behavior-preserving change and avoids per-widget stylesheet
fragments. No further safe simplification is required for D38.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material remains a transferable engineering
reference only and does not establish a private ByteDance standard,
certification, or compliance claim. The applicability record is carried in
ADR-0063.

## Authorized non-destructive validation

- `uv run python -m compileall -q src/quillforge/presentation/theme.py` — PASS.
- `uv run ruff check src/quillforge/presentation/theme.py` — PASS.
- `uv run ruff format --check src/quillforge/presentation/theme.py` — PASS.
- D38 source/QSS probe — PASS: focus selectors and semantic tokens are present;
  selected/disabled rules remain available.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No QApplication/Qt startup, screenshots, screen-reader, DPI,
  clean-machine, deployment, or hardware operation was authorized.

## Handoff and limits

The D38 handoff records the rebuilt package identity after the synchronized
package build. Release verification remains expected NO-GO while historical
runtime reports, clean-machine evidence, signing, installer, update, and
release-owner gates remain open.
