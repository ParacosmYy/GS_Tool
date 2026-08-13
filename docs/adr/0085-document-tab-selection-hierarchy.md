# ADR-0085: Document-tab selection hierarchy

- **Status:** accepted-with-limits; D60 / UI-36 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The document rail used a muted surface for selected tabs and only a bottom
accent line to distinguish the active record. Dock titles also had no leading
semantic boundary, so the shell hierarchy was harder to scan quickly across
themes and accents.

## Decision

Refine the existing centralized theme stylesheet only:

- selected tabs use the existing `selection` surface, `accent` border, and
  `accent_alt` leading indicator;
- selected-tab hover uses the existing `accent` surface and `on_accent`
  foreground while preserving the pink hover edge;
- the tab rail receives an existing `border` bottom boundary;
- dock titles receive an existing `accent_alt` leading boundary and balanced
  padding.

No new color token, widget property, signal, behavior, locale, motion, editor,
or application policy is introduced. Warning-background foreground derivation
and the existing gold endpoint contract remain unchanged.

## Invariants

1. Tab selection, focus, close, current-index, and keyboard behavior remain
   owned by the existing Qt surface.
2. All new visual values come from `ThemeColors`; no hard-coded new palette is
   introduced.
3. Selected and selected-hover foreground/background pairs meet the static
   4.5:1 threshold across all 3 themes × 4 accents.
4. Existing focus, disabled, warning, workspace, and command-rail selectors
   remain present.
5. The change is limited to `presentation.theme._stylesheet()`.

## Alternatives considered

- **Use a new selected-tab color field:** rejected; the existing selection and
  accent tokens already express the intended hierarchy.
- **Change `DocumentTabSurface` behavior or tab metadata:** rejected; this is a
  presentation-only visual distinction and needs no new state.
- **Use a gold filled selected tab:** rejected because the gold endpoint is a
  warning/action semantic and reusing it would regress the warning contract.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target and limits

- Theme source probes confirm selected/hover/focus/dock selectors and all
  existing token boundaries.
- Static contrast probes cover 12 theme/accent combinations; compile, Ruff,
  format, handoff, package identity, and expected release no-go evidence are
  recorded in the D60 handoff.
- Native QSS specificity/rendering, accessibility, DPI, font metrics, runtime
  startup, clean-machine, cross-machine, and release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
