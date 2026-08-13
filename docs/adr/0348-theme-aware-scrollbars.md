# ADR-0348: Theme-aware scrollbars

## Status

Accepted with limits — D312 / UI-130 / ARCH-282.

## Context

QuillForge's centralized QSS already establishes theme-aware panels, tabs,
inputs, separators, and focus states, but vertical and horizontal scrollbars
still inherit the native Qt/Fusion appearance. That leaves a high-frequency
editor/workspace affordance visually detached from the modern shell and makes
hover/pressed state boundaries harder to scan.

## Decision

Keep scrollbar styling in the existing `presentation.theme._stylesheet()`
owner. Use the current surface and border tokens for the track, resolve the
normal handle from `border_strong`, and resolve hover/pressed handles from
`accent_alt`/`accent` with the existing readable-edge fallback across
`surface_1` and `surface_2`. Use one compact 12-pixel track, a 28-pixel
minimum handle, rounded geometry, and hidden arrow-line controls; preserve all
Qt scrolling, range, page-step, and input behavior.

The presentation audit owns the required QSS fragments and a 3-theme/4-accent
matrix for normal, hover, and pressed handle contrast. No widget logic,
scroll model, editor adapter, locale, settings, persistence, or startup
boundary changes are introduced.

## Consequences

Scrollbars now share the shell's visual language and provide explicit state
feedback while retaining native range and interaction semantics. The static
matrix proves the token choices meet the non-text contrast floor for every
supported theme/accent combination; native style-engine painting, DPI, and
interactive drag behavior remain verification limits.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt QSS behavior and the
WCAG-style non-text contrast floor are public engineering references. No
embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer requirement, certification,
or private corporate standard claim applies.

