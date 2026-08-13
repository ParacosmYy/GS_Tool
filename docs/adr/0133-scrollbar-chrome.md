# ADR-0133: Symmetric scrollbar chrome

- **Status:** accepted-with-limits; D106/UI-54 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The centralized stylesheet had a compact vertical scrollbar, but the selector
group that hides add/sub/page controls also included `QScrollBar:horizontal`.
That applied `height: 0` to the horizontal bar itself. Long lines in the code
editor could therefore lose a visible horizontal scroll affordance or fall
back to an inconsistent native appearance.

## Decision

Keep scrollbar presentation in `presentation.theme` and define matching
vertical and horizontal track, handle, and hover states. The button/page
selector group now contains only the add/sub/page subcontrols, so the
horizontal scrollbar keeps an explicit compact height. No widget behavior,
wrap setting, scroll mode, signal, editor adapter, or application policy
changes.

## Invariants

1. Both orientations use the same surface/border/accent tokens and only
   presentation geometry changes.
2. Add-line, sub-line, add-page, and sub-page controls remain hidden; the
   scrollbar itself is not included in that group.
3. No QScintilla, EditorWidget, document, persistence, locale, or operation
   state enters the stylesheet change.
4. The change is centralized in the existing stylesheet; no widget-local QSS,
   new state owner, dependency, or asset is introduced.

## Alternatives considered

- **Keep the selector as-is:** rejected; it collapses the horizontal affordance.
- **Customize only the editor's horizontal scrollbar in EditorWidget:**
  rejected; it would split a global visual contract and couple widget code to
  theme tokens.
- **Leave native scrollbars untouched:** rejected; it preserves inconsistent
  platform rendering and the current hidden-horizontal bug.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop QSS. No MCU, embedded C/C++, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power-control, motor-control,
or manufacturer requirement applies. The mandatory embedded assurance gate is
therefore `N/A`. Public CloudWeGo material remains an engineering reference
only; no private ByteDance standard, certification, or compliance claim is
made.

## Review and simplification

- Architect: Ramanujan the 3rd / Luna max; the bounded read-only wait timed out
  and the agent was closed. Status is `NO_CONCLUSION`; no child architecture
  PASS is claimed.
- Independent review: Carson the 3rd / Luna max; the bounded read-only wait
  timed out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for selector scope, orientation symmetry, token
  reuse, and behavior preservation.
- Simplification assessment: PASS. The fix removes one incorrect selector and
  adds the missing horizontal counterpart in the existing QSS block; no new
  abstraction was introduced.

## Verification target and limits

- Required: selector/source probe, compileall, Ruff, format, package identity,
  handoff/register/index synchronization, and expected release NO-GO evidence.
- Not proven: native Qt style-engine rendering, scrollbar metrics at every DPI,
  actual horizontal scrolling, screenshots, startup, clean-machine behavior,
  signing, installer, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
