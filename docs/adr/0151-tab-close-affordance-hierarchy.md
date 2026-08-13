# ADR-0151: Document tab close-affordance hierarchy

- **Status:** accepted-with-limits; UI-62 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The document tab rail already had a visible hover and pressed treatment for
its close subcontrol, but keyboard focus and disabled states were not explicit.
The close target also had no minimum hit size in the centralized stylesheet.
That made the destructive affordance harder to find and weaker to traverse
with keyboard focus, especially across theme variants.

## Decision

Extend the existing `QTabBar#documentTabBar::close-button` QSS contract with
an 18px minimum width/height and token-driven focus and disabled states. Keep
the existing danger-colored hover/pressed behavior. The rules remain scoped to
the authored document tab bar and do not change `DocumentTabSurface`,
`tabCloseRequested`, tab ownership, close confirmation, or document policy.

## Invariants

1. `DocumentTabSurface` keeps `setTabsClosable(True)`, the existing
   `tabCloseRequested` connection, tab identity, and current-index behavior.
2. Hover and pressed close affordances retain their existing danger semantics;
   focus adds a non-destructive keyboard cue and disabled state is muted.
3. The centralized theme stylesheet remains the sole owner of these tokens;
   no widget-local stylesheet or close-policy logic is introduced.
4. The selector is scoped to `QTabBar#documentTabBar`, so unrelated tab bars
   retain their existing platform/presentation behavior.
5. Native Qt subcontrol painting, focus geometry, DPI, fonts, screen readers,
   and human visual perception remain runtime limits.

## Alternatives considered

- **Leave the existing states:** rejected; keyboard focus and disabled intent
  remain visually ambiguous.
- **Change DocumentTabSurface or add a custom close widget:** rejected; it
  would move presentation-only state into lifecycle code and widen risk.
- **Style every QTabBar close button globally:** rejected; it would affect
  unrelated surfaces and violate selector ownership.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation code. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The
mandatory embedded assurance workflow and simplifier are N/A for this source
scope; no embedded source was changed. Public CloudWeGo material remains an
engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Ampere the 4th / Luna max; two bounded read-only waits timed out
  and the agent was closed without a conclusion. Status is NO_CONCLUSION; no
  child architecture PASS is claimed.
- Independent review: Sagan the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  NO_CONCLUSION; no independent PASS is claimed.
- Parent review: PASS for selector scope, token ownership, preserved tab
  signal/lifecycle contract, and bounded state coverage.
- Simplification assessment: PASS. Three scoped QSS state rules and two
  minimum-size declarations are smaller than a custom close-button component
  or widget-level event routing.

## Verification target and limits

- Authorized evidence: source selector/token/behavior probes, compileall,
  Ruff, format, presentation-contract audit, package identity, handoff/
  register/index synchronization, no-process evidence, and expected release
  NO-GO evidence.
- Not proven: QApplication startup, native QSS subcontrol rendering,
  keyboard focus output, accessibility output, DPI/font behavior, screenshots,
  clean-machine, cross-machine, signing, installer, updater, legal, support,
  or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
