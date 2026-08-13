# ADR-0341: Localized Settings accessible names

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D305 / UI-124 / ARCH-275

## Context

The Settings dialog already translated its visible labels and behavior-control
text in `set_locale()`, but the nine value controls and three behavior controls
did not have an explicit localized accessibility-name projection. That left
assistive-technology names dependent on widget defaults and could leave names
stale after a language switch.

## Decision

Keep the existing labels and controls, and add one private
`_refresh_accessible_names()` projection at the existing `set_locale()`
boundary. The helper maps each of the nine value controls to its translated
label and maps the three behavior controls to their translated checkbox text.
`set_locale()` invokes this helper after all relevant `setText()` operations
and before the existing preview refresh.

The presentation audit verifies the helper shape, the exact 9+3 control
mapping, the two setter expressions, and the refresh ordering. No new
translation keys, settings fields, persistence data, signals, or business
behavior are introduced.

## Boundaries and compatibility

This slice changes only Qt accessibility metadata. Visible labels, locale
values, item data, current values, settings snapshots, persistence, keyboard
routing, preview behavior, and application ownership remain in their existing
paths. The names are refreshed on locale change and are not persisted.

## Evidence

- `D305-PROBE=PASS helper_loops=2 mapping_items=9 behavior_items=3 setters=2 refresh_before_preview=True locale_role=1`
- `D305-COMPILEALL=PASS`
- `D305-RUFF=PASS`
- `D305-FORMAT=PASS`
- `D305-AUDIT=PASS`
- Source startup and README file-open diagnostics passed with no window shown
  and no event loop entered.
- The rebuilt root/dist candidate has SHA-256
  `8768782D597C741B9D5D1499E153AAA73AA003D256DE97AADD661353F3A31DA1` and
  38,596,329 bytes.

## Review and applicability

The architect consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review returned `PASS` for the source/contract
scope and `NO_CONCLUSION` for full delivery signoff because it preceded the
new package/handoff records. Parent review and simplification assessment are
recorded as `PASS`.

Qt accessibility APIs and WCAG 2.2 SC 1.3.1/4.1.2 are public engineering
references for this Python 3.12/PyQt6 desktop UI. Embedded-vendor source
applicability is N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code
changed. No manufacturer, MISRA, ISO 26262, ASPICE, certification, or private
corporate-standard claim is made.

## Limits

Native EXE/Qt launch, screen-reader output, pixel rendering, focus traversal,
DPI, alternate style engines, clean-machine behavior, real DLL loading,
signing, installer, updater, and release-owner gates remain unrun under the
active non-destructive launch policy. Unit tests, mocks, fixtures, and test
harnesses were not created or run.
