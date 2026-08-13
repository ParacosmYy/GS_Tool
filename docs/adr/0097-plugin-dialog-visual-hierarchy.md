# ADR-0097: Plugin-dialog visual hierarchy

- **Status:** accepted-with-limits; D72 / UI-45 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The plugin catalog and status dialogs had semantic action roles, but their
content area still read as a mostly default Qt surface: the summary, list, and
dialog boundary did not form a clear visual hierarchy. This weakened scan
order and made the modern/cute shell feel inconsistent.

## Decision

Extend the centralized `theme.py` QSS with object-name-scoped selectors for
`pluginCatalogDialog` and `pluginStatusDialog`. Add a token-driven top accent
and boundary, a readable `dialogSummary` card, and distinct list panel,
focus, and selected-row states. Selected rows retain a left accent plus text
weight; disabled rows continue to inherit the existing muted global state.

Use `text_primary` for summary and selected-row text and reserve accents for
edges/borders, so no theme/accent combination depends on bright text over an
unknown gradient. No widget/layout/objectName/interaction policy changes.

## Invariants

1. Only centralized QSS changes; plugin dialog composition, layout, signals,
   item data, locale, and governance/runtime policy remain unchanged.
2. All three themes and four accents retain readable primary text on summary
   and selected surfaces by the static contrast probe.
3. Focus, selected, and disabled states remain distinguishable without
   relying on color alone; selected rows also gain a border and weight.
4. Native rendering remains an explicit runtime limitation under no-launch.
5. No runtime GUI launch, screenshot, or test-only asset is required or
   performed under the active no-launch policy.

## Alternatives considered

- **Add local widget stylesheets:** rejected; it would split the theme owner
  and make later theme/accent updates inconsistent.
- **Change dialog layouts or add cards:** rejected; the visual gap can be
  closed through existing object names and QSS without moving composition.
- **Use bright accent text for the whole panel:** rejected; accents are safer
  as borders, while primary text carries readability.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- The D72 theme hierarchy/contrast probe covers all 3 themes × 4 accents,
  required selectors, summary contrast, and selected-row contrast.
- Compileall, Ruff, format, handoff, package identity, and expected release
  no-go evidence are recorded in the D72 handoff.
- Architect and independent review windows returned no conclusion; no child
  PASS is claimed.
- Native QSS rendering, DPI, font metrics, accessibility, startup,
  clean-machine, cross-machine, and external release evidence remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
