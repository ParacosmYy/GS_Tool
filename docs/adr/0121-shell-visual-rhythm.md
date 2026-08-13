# ADR-0121: Shell visual rhythm and low-noise state hierarchy

- **Status:** accepted-with-limits; D96 / UI-50 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The centralized stylesheet already supplied theme, accent, focus, selection,
feedback, and warning tokens, but several ordinary shell surfaces used filled
cards, full borders, large rounding, and strong accent transitions at the same
time. That made the command rail, document tabs, status rail, and editor canvas
compete for attention instead of giving the active document the clearest visual
priority.

## Decision

Keep `theme.py` as the only stylesheet owner and refine only its shell rhythm:

- make the command rail a quiet shell row with compact tool buttons and a
  restrained context chip;
- keep the status bar and status rail quiet in the ordinary state, while
  retaining explicit success, warning, and error feedback surfaces;
- make the document-tab rail use a lighter inactive state and reserve the
  accent edge/bottom rule for the selected document and focus state;
- reduce common button rounding/padding slightly; use a flat primary accent
  instead of a two-endpoint gradient so the foreground remains governed by the
  existing contrast-aware `on_accent` token.

## Invariants

1. No widget object name, signal, shortcut, locale contract, setting, or
   application callback changes.
2. `ThemeColors`, `_best_on_accent()`, warning foreground selection, and the
   existing semantic feedback state contract remain the only color sources.
3. Normal, hover, pressed, checked, focus, selected, disabled, success,
   warning, and error states remain represented by centralized selectors.
4. Presentation remains responsible for QSS only; no document, workspace,
   persistence, or task policy enters `theme.py`.
5. The change is a bounded visual refinement, not evidence of native runtime
   rendering, accessibility, DPI, font, or release readiness.

## Alternatives considered

- **Add widget-local stylesheets:** rejected; this would split the visual
  system and make theme switching harder to audit.
- **Introduce a new theme engine or design-token package:** rejected; the
  current token dataclass and stylesheet already provide the required seam.
- **Remove all selected/feedback fills:** rejected; important states need a
  visible cue beyond text color and must remain easy to scan.
- **Keep the primary gradient:** rejected for this slice; a flat accent is
  easier to reason about and avoids another endpoint-specific text contrast
  surface.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Review and simplification

- Architect role: Goodall the 3rd / Luna max; bounded read-only window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Pascal the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for selector scope, state coverage, token reuse,
  and unchanged object-name/signal/application boundaries.
- Simplification assessment: PASS; the slice removes competing decoration
  without adding a wrapper, state model, dependency, or second stylesheet.

## Verification target and limits

- `D96-CONTRAST-PROBE=PASS` covers all three themes and four accent choices for
  the primary and gold foreground pairs.
- `D96-GRADIENT-PROBE=PASS` confirms the ordinary stylesheet no longer emits a
  gradient declaration.
- `D96-STATE-COVERAGE-PROBE=PASS` is represented by the targeted selector
  source probe for selected/focus/disabled/feedback states.
- Compileall, Ruff, format, package identity, handoff, repository, no-process,
  and expected release NO-GO checks are recorded in the D96 handoff.
- Native QSS specificity, actual layout, font metrics, accessibility, runtime
  startup, clean-machine, cross-machine, signing, installer, updater, legal,
  support, and release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
