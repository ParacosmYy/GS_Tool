# ADR-0351: Readable checkbox indicator states

## Status

Accepted with limits — D315 / UI-133 / ARCH-285.

## Context

The Settings behavior toggles and the shared checkbox surface used the
selected accent as the checked-indicator fill. Qt's native check mark remains
bound to the text foreground, so several supported theme/accent combinations
put a low-contrast mark on that accent surface. The state was technically
different but could be hard to perceive, especially for the amber/砂金
palette.

## Decision

Keep checkbox behavior and native indicator painting in Qt. In the existing
`presentation.theme._stylesheet()` owner, use the pressed and hover surfaces
for checked indicators and derive readable non-text borders with
`readable_edge_foreground_for_surfaces()`. Use the muted text/surface pair for
checked-disabled indicators. Add a 3-theme/4-accent audit for the native mark
foreground against each checked surface; do not add a custom image, event
handler, widget subclass, or settings-specific implementation.

## Consequences

Checked, checked-hover, focused, and checked-disabled checkboxes now preserve
the native tick while keeping its foreground readable on every supported
palette. Settings behavior toggles inherit the same improvement through their
existing semantic role. The accent remains visible as a readable border and
the surrounding Settings role surface; check-state, keyboard, persistence,
and accessibility semantics remain unchanged. Native pixel painting, DPI, and
screen-reader output remain runtime verification limits.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt 6 QCheckBox indicator
subcontrols/pseudo-states and the WCAG-style text contrast floor are public
engineering references. No embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer
requirement, certification, or private corporate standard claim applies.

