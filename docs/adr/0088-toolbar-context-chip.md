# ADR-0088: Toolbar context chip

- **Status:** accepted-with-limits; D63 / UI-38 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The command rail already exposed the localized “local workspace / safe by
default” context, but it rendered as muted inline text. Its trust and scope
meaning was therefore easy to miss among the command buttons.

## Decision

Refine only the existing `QLabel#toolbarContext` selector in the centralized
theme stylesheet. Reuse `surface_3`, `border`, `accent_alt`, and
`text_secondary` to render a compact chip with a leading accent boundary,
rounded edge, and balanced padding. The existing CommandSurface object,
localized text, toolbar layout, command callbacks, and application policy are
unchanged.

## Invariants

1. The change is QSS-only and scoped to the existing toolbar context label.
2. No new `ThemeColors` field, palette endpoint, i18n key, widget, signal, or
   layout owner is introduced.
3. Text remains readable against `surface_3` across all supported themes and
   accents.
4. Command rail actions, keyboard shortcuts, command refresh, and locale
   refresh remain unchanged.

## Alternatives considered

- **Use a global QLabel style:** rejected; it would change status, workspace,
  dialog, and editor labels without preserving semantic scope.
- **Add a new context state model:** rejected; the current label is static
  localized context and needs only visual hierarchy.
- **Use a new color token:** rejected; existing surface, border, accent, and
  text tokens are sufficient.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- Source selector and scope probes confirm a QSS-only context-chip change.
- Static contrast covers 3 themes × 4 accents; compile, Ruff, format, handoff,
  package identity, and expected release no-go evidence are recorded in the
  D63 handoff.
- Native QSS rendering, runtime startup, screenshots, accessibility, DPI,
  fonts, clean-machine, cross-machine, and external release evidence remain
  unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
