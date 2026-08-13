# ADR-0128: Document-tab rail state clarity

- **Status:** accepted-with-limits; UI-51 bounded visual slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The document rail used the generic `QTabBar` projection. Selected and modified
tabs were technically distinguishable, but long titles, keyboard focus, and
the native close affordance did not have a stable semantic selector or a
cohesive state treatment. A static contrast audit also found the Paper/Sand
muted text token below 4.5:1 on its light surfaces.

## Decision

Keep `DocumentTabSurface` as the sole tab widget owner and add only
presentation hints: enable document mode, assign the internal tab bar the
stable `documentTabBar` object name, use middle elision for long titles, and
prevent empty tab expansion. Add centralized `theme.py` QSS for the document
rail's normal, hover, selected, selected-hover, focus, disabled, and
close-button hover/pressed states. The modified icon and existing title marker
remain the document surface's current projection.

Adjust only the Paper/Sand `text_muted` token from `#737d8d` to `#566272`; the
new value remains visually muted while exceeding 4.5:1 against all four light
surface tokens. No other theme, accent, signal, or behavior contract changes.

## Invariants

1. `DocumentTabSurface` retains tab identity/index mapping, `tabCloseRequested`
   and `currentChanged` connections, modified icon projection, and close
   callback semantics.
2. All new selectors remain in the centralized generated stylesheet; no widget
   calls `setStyleSheet` and no second visual token source is introduced.
3. The tab bar remains keyboard-focusable and closable; middle elision changes
   only how long labels are painted.
4. The Paper/Sand correction does not change domain settings, locale, editor
   colors, persistence, or application policy.
5. Runtime QSS specificity, native metrics, DPI, font fallback, and visual
   acceptance remain explicit unrun items under the no-launch policy.

## Alternatives considered

- **Keep generic `QTabBar` styling:** rejected; it leaves close/focus states
  dependent on native style and does not provide a stable visual boundary.
- **Add per-widget stylesheet fragments:** rejected; it would split the token
  source and make theme/accent consistency harder to audit.
- **Replace the native tab bar with a custom tab widget:** rejected; it would
  change signals, close behavior, keyboard routing, and accessibility surface
  for a presentation-only problem.
- **Darken all Paper/Sand text tokens:** rejected; the static defect is isolated
  to `text_muted`, so broader palette changes would add unnecessary visual
  drift.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power-control, motor-control,
and manufacturer requirements are not applicable. The mandatory embedded
assurance gate is recorded as N/A for this change. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Review and simplification

- Architect role: Turing the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. Archimedes the 3rd /
  Luna max received a focused token-contrast confirmation; its two bounded
  waits also returned `NO_CONCLUSION`. No child architecture PASS is claimed.
- Independent review: Boyle the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No independent PASS is
  claimed.
- Parent source review: PASS for tab signal/index/dirty preservation, scoped
  QSS specificity, state completeness, and the isolated Paper/Sand contrast
  correction.
- Simplification assessment: PASS. The slice reuses the existing native tab
  surface and token stylesheet; no custom widget, state model, icon system,
  or duplicate palette was introduced. No further safe reduction was
  identified.

## Verification target and limits

- `UI-51-TAB-RAIL-SOURCE-PROBE=PASS` covers semantic tab-bar setup and required
  centralized selectors.
- `UI-51-TAB-RAIL-CONTRAST-PROBE=PASS` covers every theme/accent combination,
  tab selector presence, and the relevant text/accent pairs at 4.5:1 or
  better.
- Compileall, Ruff, format, package identity, handoff, repository,
  no-process, traceability, and expected release NO-GO evidence are recorded in
  the UI-51 handoff.
- Native Qt rendering, close-button glyph metrics, fonts, DPI, accessibility,
  runtime startup, clean-machine, cross-machine, signing, installer, updater,
  legal, support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
