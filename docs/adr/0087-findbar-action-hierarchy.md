# ADR-0087: Find-bar action hierarchy

- **Status:** accepted-with-limits; D62 / UI-37 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The find/replace bar used the global button appearance for navigation,
replacement, cancellation, and close. The controls were functional but their
visual roles were too similar, especially when the bar was in Replace mode.
The query fields also had no visual distinction from unrelated line edits.

## Decision

Keep `FindBar` as the owner of find/replace behavior and add presentation-only
object names for navigation, cancel, and close actions. Refine the existing
central theme QSS using existing tokens:

- the find bar gets an alternate-accent top edge;
- the find query uses the existing alternate accent and stronger text weight;
- the replacement query uses the existing pink accent;
- previous/next actions use a compact neutral control with a pressed/focus
  accent state;
- cancel uses the existing warning background/foreground contract on hover;
- close uses the existing error surface on hover;
- `primaryAction` and `warningAction` remain the existing semantic owners for
  the active find and Replace All actions.

No signal, callback, state-machine, locale, MainWindow, or replacement policy
changes are introduced.

## Invariants

1. Object names are presentation selectors only; all existing signal
   connections and callback routing remain unchanged.
2. No new `ThemeColors` field or hard-coded palette is introduced.
3. The `warningAction` gold endpoint and derived readable warning foreground
   remain unchanged.
4. Disabled, focus, pressed, primary, and warning states retain readable
   selectors after the more-specific find-bar rules apply.
5. The change is limited to `presentation/find_bar.py` and the centralized
   `presentation/theme.py` stylesheet.

## Alternatives considered

- **Style every global QPushButton more strongly:** rejected; it would change
  dialogs, workspace controls, and settings at once and erase action roles.
- **Add separate find-bar color tokens:** rejected; existing accent, pink,
  warning, error, surface, and text endpoints already express the hierarchy.
- **Change FindBar layout or action behavior:** rejected; the problem is
  visual scanning, not the existing interaction contract.

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

- Source probes confirm stable presentation identities, selector coverage, and
  unchanged semantic roles.
- Static contrast probes cover five new find-bar state pairs across 3 themes ×
  4 accents; compile, Ruff, format, handoff, package identity, and expected
  release no-go evidence are recorded in the D62 handoff.
- Native QSS specificity/rendering, runtime startup, screenshots, accessibility,
  DPI, fonts, clean-machine, cross-machine, and external release evidence
  remain unrun.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
