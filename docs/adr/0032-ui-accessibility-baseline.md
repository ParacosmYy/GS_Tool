# ADR-0032: Static UI accessibility baseline

- **Status:** implementation in progress for D9 UI-03/UI-04 follow-up
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

Keyboard users retain a visible focus boundary across the modern shell, and
recoverable modal errors do not leave the status rail stuck in ERROR after the
message is dismissed. The shell status also exposes explicit accessible names
and descriptions for assistive technology projections.

## Decision

1. Keep focus styling in the centralized theme. Buttons, tool buttons, line
   edits, spin boxes, combo boxes, tabs, and workspace/search lists use the
   cyan accent as their focus border or indicator; list/tree widgets no longer
   suppress their focus outline through the shared style.
2. Raise the shared muted-text token to preserve readable contrast against the
   ink surfaces while keeping the existing palette hierarchy.
3. `StatusRail` owns stable accessible names for the rail, context, and phase;
   every phase update refreshes the phase accessible description.
4. `MainWindow._show_error()` returns the presentation state to the normal
   lifecycle projection after the modal error returns. TaskRunner and close
   semantics are unchanged.

## Scope and limits

This is a source/style accessibility baseline, not a runtime WCAG, screen
reader, DPI, font, or cross-machine acceptance claim. Native platform rendering
and actual assistive-technology announcements remain unrun under the project
no-launch instruction.

## Verification

- `scripts/check.ps1` must pass formatting, compile, architecture, and
  acceptance checks.
- The source review checks centralized selectors, explicit status projection,
  and no change to application/service ownership.
- Runtime visual, native focus, contrast measurement, and screen-reader review
  remain explicit follow-up gates.

