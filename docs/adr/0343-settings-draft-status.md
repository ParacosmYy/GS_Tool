# ADR-0343: Settings unsaved-draft status

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D307 / UI-126 / ARCH-277

## Context

The Settings dialog could preview choices and restore defaults, but it did not
explicitly tell users whether the current draft differed from the values used
to open the dialog. Theme, font, animation, and behavior changes could
therefore look ambiguous before Save or Cancel.

## Decision

Capture the dialog-opening `SettingsSnapshot` as a private presentation
baseline. A localized `settingsDraftStatus` label compares the current
`settings_snapshot()` with that baseline and projects `clean` or `changed`
text/state. Existing preview controls refresh the status after their preview
work; behavior toggles refresh only the status; locale and RestoreDefaults
reuse the existing projection boundary.

The status uses explicit text plus semantic QSS state, not color alone. Its
accessible name follows the visible localized message. A small unpolish/polish
cycle reapplies the dynamic QSS property without adding a stylesheet owner.

## Boundaries and compatibility

The baseline and label exist only for one SettingsDialog lifetime. The status
is not part of `SettingsSnapshot`, SettingsService, persistence, MainWindow,
animation policy, or application ownership. Save/Cancel behavior and all
existing current-value/item-data paths remain unchanged.

## Evidence

- `D307-STATUS-PROBE=PASS baseline=1 snapshot_compare=1 preview_refresh=1 behavior_refresh=1 locale_after_preview=1 states=1`
- `D307-QSS-MATRIX=PASS themes=3 accents=4 draft_states=clean_changed minimums=clean:4.87,changed:8.79`
- `D307-COMPILEALL=PASS`
- `D307-RUFF=PASS`
- `D307-FORMAT=PASS`
- `D307-AUDIT=PASS`
- Source startup and README file-open diagnostics passed with no window shown
  and no event loop entered.
- The rebuilt root/dist candidate has SHA-256
  `86FF4639FF3D4A6CE5975EC1B0E1BD459ED540C234F23A3C8A7830FE21AE0F3B` and
  38,599,151 bytes.

## Review and applicability

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded Luna/max waits. Parent review and
behavior-preserving simplification assessment are `PASS`.

Qt dynamic properties/accessibility APIs and WCAG 2.2 are public engineering
references for this Python 3.12/PyQt6 desktop UI. Embedded-vendor source
applicability is N/A: no embedded C/C++, MCU, BSP/HAL, RTOS, or firmware code
changed. No manufacturer, MISRA, ISO 26262, ASPICE, certification, or private
corporate-standard claim is made.

## Limits

Native EXE/Qt launch, screen-reader output, pixel rendering, focus traversal,
DPI, clean-machine behavior, real DLL loading, signing, installer, updater,
and release-owner gates remain unrun under the active non-destructive launch
policy. Unit tests, mocks, fixtures, and test harnesses were not created or
run.
