# ADR-0349: Theme-aware typography steppers

## Status

Accepted with limits — D313 / UI-131 / ARCH-283.

## Context

The Settings typography controls expose interface and editor font sizes through
`QSpinBox`, but their up/down step buttons still inherit the native Qt style.
The surrounding field already has role and tone metadata, so the remaining
visual inconsistency is confined to the stepper affordance.

## Decision

Keep stepper styling in the existing `presentation.theme._stylesheet()` owner
and scope it to `QSpinBox[settingsRole="typographyChoice"]`. Use the existing
surface, border, pressed, and readable text tokens for the normal, hover,
pressed, and disabled states. Position the native `up-button` and
`down-button` subcontrols on the right with compact rounded geometry; do not
replace the arrows, intercept value changes, or alter the existing interface
and editor typography ranges.

The presentation audit owns the source fragments and a 3-theme/4-accent text
contrast matrix for the normal, hover, and pressed stepper surfaces. No
settings schema, persistence, locale, keyboard, or application boundary
changes are introduced.

## Consequences

Font-size controls now share the selected theme's visual language and make
step affordances easier to see while Qt retains its native increment/decrement
semantics. The static matrix proves the token choices meet the normal-text
contrast floor; native arrow painting, DPI, and interaction remain runtime
verification limits.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt 6 QSpinBox/QSS
subcontrol behavior and the WCAG-style text contrast floor are public
engineering references. No embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer
requirement, certification, or private corporate standard claim applies.

