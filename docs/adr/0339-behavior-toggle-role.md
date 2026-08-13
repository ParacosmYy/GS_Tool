# ADR-0339: Behavior toggle presentation role

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D303 / UI-122 / ARCH-273

## Context

The Settings dialog exposed wrapping, line-number, and motion checkboxes as
three unrelated concrete selectors. That made the behavior controls harder to
extend and allowed editor/interface cues to drift as states were added. The
existing values, signals, snapshot reads, persistence, and keyboard behavior
are already stable and must remain owned by their current modules.

## Decision

Keep the three existing `QCheckBox` instances and assign presentation-only
dynamic properties before their existing checked-value setup:

- `settingsRole="behaviorToggle"` is the shared semantic role.
- `settingsTone="editor"` marks wrapping and line numbers.
- `settingsTone="interface"` marks motion.

`presentation.theme` owns one role-based normal/hover/focus/checked/disabled
QSS contract. Tone selectors retain the left-edge distinction for every state,
including disabled. `presentation.theme_tokens` owns a pure helper that keeps a
preferred edge only when it reaches the 3:1 non-text floor across all surfaces
used by the state machine; otherwise it uses the existing accent-gold fallback.

The presentation audit checks declaration order, required states, tone-specific
disabled cues, and removal of the old active ID selectors.

## Boundaries and compatibility

This slice does not change settings schema, values, persistence, locale,
editor projection, motion policy, signals, keyboard routing, or application
ownership. Existing object names, labels, checked values,
`SettingsSnapshot` reads, generic indicators, and disabled behavior remain
compatible. `settingsRole` and `settingsTone` are not persisted.

## Evidence

- `D303-BEHAVIOR-TOGGLE-ROLE=PASS controls=3 role=behaviorToggle tones=editor_interface legacy_active_ids=0`
- `D303-BEHAVIOR-TOGGLE-CONTRACT=PASS checked_values_snapshot_indicators_signals_unchanged=1`
- `D303-QSS-MATRIX=PASS themes=3 accents=4 states=normal_hover_focus_checked_disabled minimum_text=5.14 edge=3.02`
- Source startup and README file-open diagnostics passed without showing a
  window or entering the event loop.
- The rebuilt root/dist candidate has SHA-256
  `14C4791E265CDF30A4C012F6F52301192AEDF9A47D8D9B3E7A8B286C8984289D` and
  38,595,633 bytes.

## Review and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
waits. The independent code review also returned `NO_CONCLUSION` after three
bounded waits; no independent PASS is claimed. Parent review is `PASS` and the
simplification assessment is `PASS`.

Qt Style Sheets and WCAG 2.2 SC 1.4.11 are public engineering references for
this Python 3.12/PyQt6 desktop UI. Embedded-vendor source applicability is
N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code changed. No
manufacturer, MISRA, ISO 26262, ASPICE, certification, or private corporate
standard claim is made.

## Limits

Native EXE/Qt launch, pixel rendering, focus/accessibility, DPI, clean-machine
behavior, real DLL loading, signing, installer, updater, and release-owner
gates remain unrun under the active non-destructive launch policy. Unit tests,
mocks, fixtures, and test harnesses were not created or run.
