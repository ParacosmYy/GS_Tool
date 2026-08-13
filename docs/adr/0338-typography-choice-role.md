# ADR-0338: Give Settings typography controls one semantic choice role

## Status

Accepted with limits — D302 / UI-121 / ARCH-272.

## Context

The Settings dialog already supports interface/editor font family, size, and
style values, but their QSS hierarchy was split across six concrete object-ID
groups. That made the controls harder to extend consistently and left the
interface/editor distinction encoded in selector duplication rather than in a
presentation meaning.

## Decision

Keep every existing typography object name, value source, font preview,
signal, locale route, persistence contract, keyboard path, and snapshot read.
Before value population or range/style configuration, assign each control:

- `settingsRole="typographyChoice"`
- `settingsTone="interface"` for UI font family/size/style
- `settingsTone="editor"` for editor font family/size/style

Centralized `presentation.theme` now owns the shared typography normal,
hover, focus, combo-open, and disabled states through the role. Tone-specific
edges preserve the interface/editor visual cue. Open-state edge colors use the
existing `ThemeColors` palette plus one Qt-free
`readable_edge_foreground()` helper with a 3:1 non-text contrast floor; the
3-theme × 4-accent audit verifies the resolved result. The disabled selector
is placed after active states so it remains the final compatibility boundary.

No widget-local stylesheet, new settings schema, custom control, second color
policy, or application ownership seam is introduced.

Qt's dynamic-property stylesheet guidance is an applicable framework
reference: https://doc.qt.io/qt-6/stylesheet-syntax.html. The 3:1 edge check
is an engineering accessibility floor informed by WCAG 2.2 SC 1.4.11:
https://www.w3.org/TR/WCAG22/#non-text-contrast. Neither is a manufacturer
requirement.

## Consequences

- Future typography controls can join one semantic QSS contract without
  duplicating six concrete selectors.
- Interface and editor controls remain visually distinguishable in normal,
  focus, hover, and open states.
- A low-contrast accent edge falls back to the existing accent-gold endpoint
  for the pressed surface; the semantic role remains visible without relying
  on an unreadable border.
- Native Qt polish timing, rendering, focus/accessibility, DPI/font fallback,
  clean-machine behavior, and release gates remain unverified under the active
  `software_start_allowed=false` policy.

## Public-source applicability

This is Python 3.12/PyQt6 desktop presentation code. Qt documentation and
WCAG are public engineering references; public embedded-vendor source
applicability is N/A. No embedded C/C++, MCU, BSP/HAL, RTOS, manufacturer
requirement, MISRA, ISO 26262, ASPICE, certification, or private
ByteDance-standard claim is made.

## Verification boundary

Required evidence is the semantic-role source contract, QSS selector/value
audit, 3-theme × 4-accent text and edge contrast matrix, compile/Ruff/format,
source startup/file-open diagnostics, package identity, PE/archive inspection,
project/handoff checks, parent review, independent review record, and the
expected release no-go result. Native EXE/Qt startup remains outside the
authorized validation boundary.
