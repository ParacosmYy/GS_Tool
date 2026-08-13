# ADR-0063: Focus-state visibility in the central QSS

- **Status:** accepted-with-limits; D38 / UI-24 / ARCH-28 bounded visual slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shell already had semantic hover, selected, pressed, disabled, and focus
selectors, but focus was often communicated only by a border-color change.
That made keyboard navigation difficult to scan on the command rail, ordinary
tool buttons, document tabs, and settings checkboxes—especially when the
surface and border values were close. The existing application already owns
keyboard routing and focus; this is a presentation-only clarity gap.

## Decision

Strengthen the existing centralized QSS focus projection in
`presentation/theme.py`:

- focused command-rail and ordinary tool buttons receive the existing hover
  surface and primary foreground in addition to the accent-alt border;
- focused document tabs receive the hover surface, primary foreground, and an
  accent-alt boundary while retaining the bottom focus marker;
- focused checkboxes receive a restrained surface cue and their indicator uses
  the accent-alt border, without changing checked/disabled semantics.

No new widget state, signal, event filter, layout owner, application contract,
or animation is introduced. Existing selected, pressed, checked, and disabled
selectors remain the stronger state owners where they apply.

## Invariants and boundaries

1. `presentation/theme.py` remains the single QSS/token owner.
2. Focus remains native Qt focus; no text parsing or application state is
   inferred from visual properties.
3. Command callbacks, tab/document routing, settings persistence, locale,
   theme selection, and motion preference are unchanged.
4. No color is the sole source of meaning for consequential application
   outcomes; this slice only improves keyboard focus visibility.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware. MCU
vendor requirements are not applicable. Public CloudWeGo pages remain
transferable engineering references only; they do not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Source probe confirms the four focus selector families and their semantic
  tokens, with selected/disabled rules still present.
- Compile, Ruff, format, handoff, package identity, and release no-go evidence
  are recorded.
- No QApplication/Qt startup, screenshot, unit test, or test-only asset is
  created or run under the active policy.

## Limits

Native QSS specificity, actual focus geometry, font metrics, screen-reader
announcements, DPI, cross-machine appearance, and user-interactive keyboard
acceptance remain unrun. The slice does not claim a complete visual redesign.
