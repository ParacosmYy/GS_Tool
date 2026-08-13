# ADR-0336: Give pending theme and accent choices a stronger visual hierarchy

## Status

Accepted with limits — D300 / UI-119 / ARCH-270.

## Context

The Settings dialog already exposes language, theme, accent, typography, and
motion choices, but the theme and accent selectors are visually close to the
ordinary font and size controls. Their swatch icons communicate the values,
yet the closed controls do not establish a strong visual anchor for the two
choices that change the shell's identity. This makes scanning and keyboard
focus less obvious in a dense settings surface.

## Decision

Keep the existing `settingsTheme` and `settingsAccent` object names and apply
their hierarchy in the centralized `presentation.theme` QSS projection only.
The two selectors receive a `surface_3` closed state, a three-pixel accent
edge, a semibold/strong label weight, and explicit hover, focus, and open-menu
states. Existing generic combo-box rules continue to own the drop-down,
item-view, disabled, and keyboard behavior.

No settings value, `UserRole`/`currentData()` contract, icon refresh, signal,
locale, persistence, preview, application service, or composition ownership
moves. The presentation contract audit guards the selector identities and the
semantic state rules.

Qt 6.11.1's versioned `Qt Style Sheets Reference`, QComboBox box-model and
pseudo-state sections, is the applicable framework source; it is an
engineering reference for this desktop UI, not a manufacturer requirement:
https://doc.qt.io/qt-6/stylesheet-reference.html

## Consequences

- Theme and accent choices are easier to locate and distinguish from font
  controls without introducing another widget or style system.
- Hover, keyboard focus, and an open combo menu retain visible non-textual
  state cues through background and border changes.
- Existing item selection and disabled-state rules remain shared and
  centralized.
- Native Qt painting, font metrics, accessibility tree behavior, DPI scaling,
  and runtime visual acceptance remain unrun under the active
  `software_start_allowed=false` policy.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. The Qt Company, `Qt Style
Sheets Reference`, Qt 6.11.1, QComboBox box-model and pseudo-state sections,
https://doc.qt.io/qt-6/stylesheet-reference.html, is an engineering reference
with desktop Qt scope. No embedded C/C++, MCU, BSP/HAL,
RTOS, manufacturer requirement, MISRA, ISO 26262, ASPICE, certification, or
private ByteDance-standard claim applies.

## Verification boundary

Compile, Ruff, format, presentation-contract audit, source startup/file-open
diagnostics, theme/accent matrix QSS probing, package identity, PE/archive
inspection, project/handoff checks, and expected release no-go verification
are required. Native rendering and clean-machine behavior remain unrun.
