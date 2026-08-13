# ADR-0230: Status accessibility localization

## Status

Accepted with limits for D181 / UI-93 / ARCH-168.

## Context

The shell already translated visible status labels and transient notification
text, but the four stable accessibility names/descriptions owned by the status
rail were hardcoded in English. Changing the application locale therefore did
not fully reproject the status surface for screen readers or accessibility
inspection.

## Decision

1. Add four stable presentation-only i18n keys for shell status, workspace
   context, shell phase, and shell notification, plus one phase-description
   template.
2. Keep `StatusRail` responsible for its Qt object tree and call the existing
   `set_locale`/`set_phase` projection path so names and the dynamic phase
   description always use the current locale.
3. Keep `StatusSurface` responsible for the notification label and refresh its
   accessible name in the same locale path that already reprojects visible
   notification text.
4. Do not change notification payloads, severity, timers, lifecycle policy,
   theme QSS, or application/domain contracts.

## Public-source applicability

This is a Python 3.12/PyQt6 presentation change. Python/Qt accessibility and
QObject ownership are engineering references already recorded in the
enterprise architecture specification, not manufacturer requirements. Public
CloudWeGo material is an engineering reference only; no private ByteDance
standard, certification, or compliance claim is made. Embedded C/C++, MCU,
RTOS, BSP/HAL/CMSIS, ISR/DMA, boot/OTA, Flash/NVM, power, and motor-control
requirements are not applicable.

## Review and verification

- Architect window: Bacon the 5th / Luna max — `NO_CONCLUSION` after the
  bounded wait; no child PASS is claimed.
- Independent review window: Hume the 5th / Luna max — `NO_CONCLUSION` after
  the bounded wait; no child PASS is claimed.
- Parent review: `PASS`.
- Simplification assessment: `PASS`; the change reuses the existing i18n and
  locale-refresh boundaries and removes no behavior or error handling.
- Authorized non-destructive evidence: bilingual accessibility-key probe,
  source wiring probe, `compileall`, Ruff, format, project check, packaging,
  manifest identity, handoff verification, and expected release no-go dossier.
- GUI/QApplication startup, screenshots, screen-reader execution, native
  rendering, clean-machine, cross-machine, and release-owner evidence remain
  unrun under project policy.
