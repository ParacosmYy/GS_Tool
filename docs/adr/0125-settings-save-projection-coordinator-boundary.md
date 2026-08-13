# ADR-0125: Settings-save projection coordinator boundary

- **Status:** accepted-with-limits; D100 / ARCH-74 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D83 moved settings-save completion classification into
`SettingsSaveCoordinator`, while the valid result still ran a mixed Qt-shell
sequence in `MainWindow`: persist the in-memory snapshot, optionally apply the
QApplication theme, retranslate the shell, update open editors, animate the
transition, and notify success. That kept callback classification separate but
left the valid settings projection order hidden in the composition root.

## Decision

Extract the valid result sequence into the Qt-free
`SettingsSaveProjectionCoordinator`. It receives a `SettingsSnapshot` already
validated by D83 and invokes explicit callbacks in the existing order:

1. apply the snapshot and optional QApplication/theme baseline;
2. retranslate the shell;
3. apply editor settings to open tabs;
4. animate the optional theme transition; and
5. publish the success notification.

`SettingsSaveCoordinator` remains responsible for tracker identity, stale
suppression, `SettingsSnapshot` validation, and invalid/failure projection.
MainWindow remains the composition root and supplies concrete QApplication,
theme, locale, editor, motion, notification, settings-service, and close-policy
callbacks.

## Invariants

1. `settings_save_projection_coordinator.py` imports no PyQt6, QApplication,
   QSS, font, widget, settings service, or persistence implementation.
2. The coordinator is called only after D83 accepts a matching valid snapshot;
   it does not validate, repair, retry, or swallow callback exceptions.
3. If no QApplication exists, the snapshot still becomes the in-memory settings
   baseline and the remaining retranslate/editor/motion/notification order is
   unchanged; only the concrete theme callback is a no-op.
4. Locale, font, theme, accent, and motion values are passed through existing
   MainWindow/settings/editor policy without normalization or schema changes.
5. No settings persistence, close guard, command contract, theme token, or
   runtime visual claim changes.

## Alternatives considered

- **Keep `_apply_saved_settings` in MainWindow:** rejected; valid settings
  ordering remains coupled to the Qt shell after D83 has isolated completion
  classification.
- **Expand SettingsSaveCoordinator with QApplication/editor behavior:**
  rejected; it would combine tracker classification and concrete projection,
  weakening the existing D83 boundary.
- **Create a universal preference or document-operation coordinator:**
  rejected; it would add broad indirection for unrelated settings and document
  contracts.
- **Move QApplication/theme/editor implementations into the new module:**
  rejected; explicit callbacks preserve presentation ownership and keep the
  new module framework-neutral.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation/application-boundary code. Embedded
C/C++, MCU, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance gate is recorded as N/A for this change.
Public CloudWeGo material remains an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made.

## Review and simplification

- Architect role: Dalton the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No child architecture
  PASS is claimed.
- Independent review: Hooke the 3rd / Luna max; two bounded read-only waits
  returned `NO_CONCLUSION`, then the agent was closed. No independent PASS is
  claimed.
- Parent source review: PASS for exact settings/theme/locale/editor/motion/
  notification ordering, QApplication optional behavior, and D83 boundary
  retention.
- Simplification assessment: PASS. The extraction names one settings projection
  contract and keeps concrete Qt callbacks at the composition root; no second
  settings model, service locator, or generic preference framework was added.
  No further safe behavior-preserving reduction was identified.

## Verification target and limits

- `D100-SETTINGS-PROJECTION-SOURCE-PROBE=PASS` covers the Qt-free source
  boundary, direct D83 wiring, and removal of the old MainWindow method.
- `D100-SETTINGS-PROJECTION-ORDER-PROBE=PASS` covers snapshot/theme,
  retranslation, editor settings, motion, and success-notification order.
- Compileall, Ruff, format, package identity, handoff, repository, no-process,
  traceability, and expected release NO-GO evidence are recorded in the D100
  handoff.
- Native settings dialog, QApplication/theme rendering, font metrics,
  animation timing, accessibility, DPI, runtime startup, clean-machine,
  cross-machine, signing, installer, updater, legal, support, and release-owner
  evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
