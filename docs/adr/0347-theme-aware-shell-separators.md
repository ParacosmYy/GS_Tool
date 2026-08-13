# ADR-0347: Theme-aware shell separators

## Status

Accepted with limits — D311 / UI-129 / ARCH-281.

## Context

The main window and dock panels already use QuillForge's centralized QSS
renderer, but their resize separators still rely on the default Qt/Fusion
painting. That leaves a low-salience boundary without the selected theme's
edge hierarchy or a clear hover cue, which weakens the modern shell and makes
panel boundaries harder to scan.

## Decision

Keep separator styling in the existing `presentation.theme._stylesheet()`
owner. Resolve a normal separator foreground from the existing border token
with `readable_edge_foreground_for_surfaces()` across `surface_0` through
`surface_2`, falling back to `text_primary` when the border is not strong
enough. Apply the same one-pixel rule to `QMainWindow::separator` and
`QDockWidget::separator`, and use the existing `accent_alt` token for the
hover state.

The presentation audit owns the source fragments and a 3-theme/4-accent
contrast matrix. No widget, layout, persistence, command, locale, or startup
boundary changes are introduced.

## Consequences

The shell has a consistent, visible split boundary and a theme-aware
interaction cue without duplicating styling in individual panels. The normal
edge is guaranteed to meet the non-text contrast floor for all three shell
surfaces, and the hover edge is checked against the same surfaces. Native Qt
painting, DPI behavior, alternate style engines, and packaged interaction
remain verification limits because native startup is not authorized in this
checkout.

## Applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt QSS behavior and the
WCAG-style non-text contrast floor are public engineering references. No
embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer requirement, certification,
or private corporate standard claim applies.

