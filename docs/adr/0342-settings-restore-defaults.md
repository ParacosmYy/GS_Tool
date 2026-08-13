# ADR-0342: Settings draft restore defaults

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D306 / UI-125 / ARCH-276

## Context

Settings already offered language, theme, accent, typography, and motion
choices, but a user who had tuned several values had no fast, reversible way
to return the dialog to the product defaults. Recovery should not silently
persist or apply a partial snapshot.

## Decision

Add `QDialogButtonBox.StandardButton.RestoreDefaults` to the existing Settings
action rail and bind it to a private `SettingsDialog._restore_defaults()`
projection. The method copies values from the existing immutable
`DEFAULT_SETTINGS`, blocks widget signals during the batch update, refreshes
palette icons, and calls the existing locale/preview projection boundary.

The operation changes only the current dialog draft. It does not call Save,
the settings service, `accept()`, or `reject()`. Existing Save/Cancel semantics
therefore remain the persistence and discard boundaries. The reset action has
localized text, a localized explanatory tooltip, an accessible name, and a
warning-tone QSS state using existing theme tokens.

The presentation audit guards the RestoreDefaults button role, the 12-control
coverage, signal blocking, draft-only behavior, locale ordering, and all reset
button QSS states.

## Boundaries and compatibility

No SettingsSnapshot field, schema, service, port, domain model, translation
architecture, animation policy, or MainWindow ownership changes. Resetting to
the default `zh-CN` locale reuses `set_locale()` and retains the existing
selection/data/accessibility/preview paths. Persistence still occurs only
through the existing Save admission after the dialog is accepted.

## Evidence

- `D306-RESET-PROBE=PASS controls=12 missing=0 restore_defaults=1 signal_blocker=1 draft_only=1 locale_before_accessibility=1 qss=1`
- `D306-QSS-MATRIX=PASS themes=3 accents=4 reset_states=normal_hover_focus_pressed_disabled minimums=normal:4.87,hover_focus:8.79,pressed:8.64,disabled:5.14`
- `D306-COMPILEALL=PASS`
- `D306-RUFF=PASS`
- `D306-FORMAT=PASS`
- `D306-AUDIT=PASS`
- Source startup and README file-open diagnostics passed with no window shown
  and no event loop entered.
- The rebuilt root/dist candidate has SHA-256
  `D8D287F7D15BB2F599742DB9299D49DDCB7171B754180BD3A4B6237188EA092F` and
  38,598,403 bytes.

## Review and applicability

The architecture consultation returned `NO_CONCLUSION` after three bounded
Luna/max waits. The independent review also returned `NO_CONCLUSION` after
three bounded waits. Parent review and behavior-preserving simplification
assessment are `PASS`.

Qt QDialogButtonBox/accessibility APIs and WCAG 2.2 are public engineering
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
