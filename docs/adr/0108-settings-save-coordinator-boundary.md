# ADR-0108: Settings-save coordinator boundary

- **Status:** accepted-with-limits; D83 / ARCH-58 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` contained both settings-save callback classification and the
settings application consequences. `SettingsSaveTracker` already owned the
positive operation identity and stale/invalid/valid/failure classification,
but the worker callbacks still entered the shell directly.

## Decision

Add the Qt-free `SettingsSaveCoordinator`. It consumes the existing tracker,
accepts explicit callbacks for applying a valid `SettingsSnapshot`, projecting
an invalid result, and projecting a worker failure, and preserves the
TaskRunner callback shape `(result, operation_id)` / `(error, operation_id)`.

Keep MainWindow responsible for `SettingsService`, candidate editing,
TaskRunner submission, `_settings` mutation, `QApplication` theme projection,
locale retranslation, editor font/settings refresh, transition timing,
notifications/error dialogs, and close policy. The coordinator does not know
about Qt or move settings policy into a generic module.

## Invariants

1. `presentation/settings_save_coordinator.py` imports no PyQt6 or widget type.
2. A stale success or failure callback has no user-facing side effect.
3. A matching invalid result projects the existing `Settings failed` error and
   does not mutate the active settings snapshot.
4. A matching valid `SettingsSnapshot` applies settings in the existing order:
   state, application theme, UI retranslation, editor refresh, transition,
   then success notification.
5. A matching worker failure projects the existing error text and does not
   apply a partial settings candidate.
6. SettingsService, settings in-flight state, plugin/recovery close gates,
   startup policy, and TaskRunner ownership remain in MainWindow.

## Alternatives considered

- **Leave both callbacks in MainWindow:** rejected; tracker classification and
  callback routing remain a focused framework-neutral boundary.
- **Move settings application into the coordinator:** rejected; Qt theme,
  locale, font, editor, transition, and message policy are presentation
  consequences owned by MainWindow.
- **Inject a large settings view/service object:** rejected; three narrow
  callbacks make the contract explicit without introducing a service locator
  or a second settings policy owner.
- **Remove SettingsSaveTracker:** rejected; it remains the canonical operation
  identity and stale-callback guard used by close handling.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power,
motor-control, and manufacturer requirements are not applicable. Public
CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Beauvoir the 3rd / Luna max; bounded window returned
  `NO_CONCLUSION`, so no child architecture PASS is claimed.
- Independent review: Aquinas the 3rd / Luna max; bounded read-only window
  returned `NO_CONCLUSION` and was closed, so no independent PASS is claimed.
- Parent source review: PASS for tracker identity, callback order, exact
  settings side-effect ordering, stale/invalid/failure behavior, and policy
  retention.
- Simplification assessment: two MainWindow worker callbacks become one
  focused coordinator while three narrow policy seams keep application of
  settings in the existing owner. No further behavior-preserving reduction
  was identified.

## Verification target and limits

- `D83-SETTINGS-SAVE-BOUNDARY-PROBE=PASS` covers callback wiring, removed old
  callbacks, settings policy retention, and close gates.
- `D83-SETTINGS-SAVE-QT-FREE-PROBE=PASS` confirms the coordinator imports
  without PyQt6 through the bare Python path.
- Targeted compileall, Ruff, format, package identity, handoff, repository
  checks, and expected release no-go evidence are recorded in the D83 handoff.
- Native Qt timing, settings dialog interaction, font availability, theme
  rendering, accessibility, runtime startup, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence
  remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
