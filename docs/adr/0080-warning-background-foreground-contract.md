# ADR-0080: Warning-background foreground contract

- **Status:** accepted-with-limits; D55 / UI-35 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shared stylesheet used the decorative `accent_gold` endpoint as the text
foreground for warning-background states in the transient status message,
attention phase, and workspace cancel affordance. That couples readable text
to a saturated theme endpoint and was reported as a visibly broken 砂金 state.

## Decision

Derive one local `warning_foreground` inside `_stylesheet()` with the existing
pure `_best_on_accent()` helper and the actual `warning_bg` token. Use it for
all warning-background text selectors: the transient status message,
attention phase, workspace status/search feedback, workspace cancel
hover/focus, warning action, and Find status. Keep `accent_gold` for
warning borders and keep `on_accent_gold` for the gold-filled warning-action
hover state.

No new `ThemeColors` field, widget-local stylesheet, runtime contrast engine,
signal, command, or application policy is introduced.

## Invariants

1. Every supported theme/accent combination selects a foreground with a
   static contrast ratio of at least 4.5:1 against `warning_bg`.
2. All warning-background text selectors use the derived warning foreground
   rather than the decorative gold endpoint or a second generic foreground.
3. Gold borders, warning-action hover, settings, locale, motion, and command
   behavior remain unchanged.
4. Contrast derivation remains in the centralized presentation theme module;
   no lower layer imports Qt or theme policy.

## Alternatives considered

- **Keep `accent_gold` as warning text:** rejected because the text endpoint
  is not the same semantic role as a warning-background foreground.
- **Add a second theme engine or widget-local fix:** rejected because it would
  duplicate token ownership and make future themes harder to audit.
- **Add a persistent `on_warning` field immediately:** deferred; the local
  derivation is the smallest complete fix and avoids expanding the settings or
  theme schema without a second consumer.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; it does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target and limits

- A pure source probe covers all three themes and four accent selections,
  generated warning selectors, and the 4.5:1 threshold.
- Compile, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the UI-35 handoff.
- QApplication/Qt startup, screenshots, native style rendering, DPI, fonts,
  accessibility, and cross-machine appearance remain unrun under the active
  no-launch policy.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
