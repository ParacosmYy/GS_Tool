# ADR-0089: Settings control hierarchy

- **Status:** accepted-with-limits; D64 / UI-39 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The settings dialog already supported language, theme, accent, UI font/size,
editor font/size, wrapping, line numbers, and motion, but both groups and all
selection controls shared generic presentation. The configuration surface did
not visually explain which controls belong to appearance versus editor
behavior.

## Decision

Add presentation-only object names to the existing appearance and editor
`QGroupBox` instances. In the centralized stylesheet:

- appearance and editor groups receive alternate-accent and pink left
  boundaries respectively;
- both group titles keep the readable `text_primary` endpoint;
- theme/UI-font/UI-size controls receive an alternate-accent boundary;
- accent/editor-font/editor-size controls receive a pink boundary.

The `SettingsSnapshot` contract, control ranges and values, language-change
signal, `SettingsSurface`, persistence service, MainWindow policy, and locale
projection remain unchanged.

## Invariants

1. Object names are presentation selectors only; no settings state or signal is
   moved.
2. Existing `ThemeColors` are reused; no token or palette is added.
3. Group-title and control text remain readable across every supported theme
   and accent; accent colors are used as boundaries, not low-contrast body text.
4. The change is limited to `settings_dialog.py` presentation metadata and
   centralized theme QSS.

## Alternatives considered

- **Use accent-colored group titles as text:** rejected after the static probe
  found Paper/Sand title contrast below 4.5:1; accents remain boundaries.
- **Add a settings preview/state model:** rejected; the requested gap is visual
  grouping and no new state is needed.
- **Change SettingsSurface or persistence ownership:** rejected; it would mix
  presentation with settings application policy.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- Source probes confirm presentation object names, selector scope, and no
  settings-policy changes.
- Contrast covers 3 themes × 4 accents × 4 settings states; compile, Ruff,
  format, handoff, package identity, and expected release no-go evidence are
  recorded in the D64 handoff.
- Native QSS rendering, runtime settings interaction, screenshots,
  accessibility, DPI, fonts, clean-machine, cross-machine, and external
  release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
