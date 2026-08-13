# ADR-0166: Editor-stage visual rhythm

- **Status:** accepted-with-limits; UI-66 / ARCH-104 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

UI-64 elevated the command rail and document-tab rail, but the central
`EditorShellSurface` still composed its tab surface and hidden/shown Find
surface with zero content margins and zero spacing. The surrounding shell and
the editor surfaces therefore read as one dense strip instead of a deliberate
workspace stage.

## Decision

Let `EditorShellSurface` own a small, explicit stage rhythm: 10px horizontal
and 8px vertical content margins, with 8px between the document-tab surface
and the Find surface. Change the existing `QWidget#editorShell` selector to
use `ThemeColors.surface_1`, leaving the tab/editor surfaces on their existing
`surface_0` canvas. This creates depth using the existing token system without
adding a second styling layer.

## Invariants

1. The tab surface remains the first child and the Find surface remains the
   second child of the same `QVBoxLayout`.
2. Find visibility, locale projection, tab signals, callbacks, document
   policy, and MainWindow composition remain unchanged.
3. No new token, widget, object name, signal, animation, font, or application
   state is introduced.
4. The stage geometry is owned by `EditorShellSurface`; color ownership stays
   in centralized `theme.py` and all existing themes/accent choices continue
   to resolve through `ThemeColors`.

## Alternatives considered

- **Keep zero margins:** rejected; it preserves the dense, legacy-like shell
  hierarchy reported by the user.
- **Add padding or per-widget margins in QSS:** rejected; native layout
  geometry is clearer and more portable when owned by the composing surface.
- **Create a new editor-stage widget or token family:** rejected; the existing
  shell is already the correct cohesive boundary and does not need a new
  abstraction for four spacing values.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation composition and QSS. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Chandrasekhar the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`.
- Independent review: Hume the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for layout ownership, child order, token scope, and
  unchanged tab/Find behavior boundaries.
- Simplification assessment: PASS. Existing shell composition and centralized
  token stylesheet are the smallest cohesive seam; no new widget or style
  abstraction is necessary.

## Verification target and limits

- Authorized evidence: shell/theme source-contract probe, compileall, Ruff,
  format, package identity, root/dist identity, no-process evidence,
  handoff/register/index synchronization, and expected release NO-GO evidence.
- Not proven: native Qt rendering, QApplication startup, DPI/font metrics,
  accessibility, runtime interaction, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
