# ADR-0350: Theme-aware combo-box expanded affordance

## Status

Accepted with limits — D314 / UI-132 / ARCH-284.

## Context

The language, theme, accent, font-family, and font-style settings are all
presented by `QComboBox`. Their normal, hover, pressed, disabled, popup-item,
and arrow states already use the centralized theme stylesheet, but the
drop-down subcontrol did not receive the popup-open (`:on`) surface state.
That left the field body and arrow state visually stronger than the narrow
button that actually indicates the open menu.

## Decision

Keep the fix in `presentation.theme._stylesheet()`, the existing owner for
shared Qt presentation tokens. Position the native `QComboBox::drop-down`
subcontrol explicitly against the border rectangle, and add a shared `:on`
state using the existing pressed surface and accent border tokens. Retain the
native `QComboBox::down-arrow`, popup model, keyboard navigation, item data,
settings roles, and locale/theme/font behavior. Extend the presentation audit
with a 3-theme/4-accent contrast matrix for the expanded-state foreground.

## Consequences

Every current and future combo box using the shared stylesheet gets a clearer
expanded affordance without widget-local styling or behavior coupling. The
explicit subcontrol geometry is stable across the existing theme owner while
Qt retains popup and selection semantics. Native rendering, DPI metrics, and
actual popup interaction remain runtime verification limits.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt 6 QComboBox
subcontrols/pseudo-states and the WCAG-style text contrast floor are public
engineering references. No embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer
requirement, certification, or private corporate standard claim applies.

