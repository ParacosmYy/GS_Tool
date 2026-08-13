# ADR-0176: settings-save admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D136 / ARCH-114

## Context

`MainWindow._show_settings` still combined settings-service availability,
in-flight admission, modal candidate editing, operation reservation, tracker
binding, and asynchronous `SettingsSaveCoordinator` submission. The existing
completion and projection coordinators already own result and application
policy, so the admission sequence was the remaining composition-root seam.

## Decision

Add the Qt-free `SettingsSaveAdmissionCoordinator` and frozen/slotted
`SettingsSaveAdmissionPorts` contract. The coordinator owns only this order:

1. capture the composition-bound settings persistence service;
2. reject missing persistence with the existing error notification;
3. reject an in-flight save with the existing warning notification;
4. invoke the existing settings editor and stop on dialog cancellation;
5. reserve the existing generic operation identity;
6. bind it through `SettingsSaveTracker`; and
7. submit the captured service operation through the existing
   `SettingsSaveCoordinator` and `TaskRunner` dispatcher.

Settings validation/persistence, `SettingsSaveCoordinator` result
classification, `SettingsSaveProjectionCoordinator`, QApplication theme
application, locale/font/editor refresh, motion transition, notifications,
and close policy remain in their existing owners.

## Alternatives rejected

- Keeping the sequence in `MainWindow` leaves settings admission coupled to the
  composition root and makes the save path inconsistent with the other typed
  admission boundaries.
- Moving the dialog, normalization, result classification, or theme/locale/
  font application would merge UI, persistence, completion, and projection
  policies.
- A generic event bus or variadic callback port would hide the tracker and
  operation ordering for a single settings-save path.

## Review and evidence

Confucius the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Descartes the 4th /
Luna max was assigned the independent read-only review and also returned no
conclusion; no child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because one focused typed admission boundary replaces the
composition-root sequence without duplicating settings completion or
application policy.

Authorized non-destructive evidence: `D136-SETTINGS-ADMISSION-BEHAVIOR-
PROBE=PASS`, `D136-SETTINGS-ADMISSION-CONTRACT-PROBE=PASS`,
`D136-QT-FREE-SETTINGS-PROBE=PASS`, `D136-COMPILEALL=PASS`,
`D136-RUFF=PASS`, `D136-FORMAT=PASS`, project checks, package identity,
no-launch, traceability, and expected release NO-GO. Native settings dialog,
queued worker timing, persistence durability, theme/font/locale/motion
rendering, clean-machine, cross-machine, and release-owner evidence remain
unrun under the active boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
