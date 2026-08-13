# ADR-0076: settings-save callback boundary

- **Status:** accepted-with-limits; D51 / ARCH-41 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

Settings persistence was represented by a MainWindow boolean. The save dialog
was already single-flight, but completion handlers ignored the operation ID,
so the lifecycle contract did not state how a stale or duplicate callback
should behave. The boundary must preserve validated `SettingsSnapshot`
results, theme/locale/font/editor application, transition animation, success
and failure notifications, and close-event waiting.

## Decision

Add the Qt-free `presentation.settings_save_tracker.SettingsSaveTracker`.
It owns only one positive TaskRunner operation ID and classifies matching
callbacks as `valid`, `invalid`, or `stale`; matching failures release the
operation. MainWindow remains responsible for dialog editing, SettingsService,
TaskRunner dispatch, applying the persisted snapshot, theme/locale/font/editor
projection, animation, error/success notifications, and close policy.

## Invariants

1. Only one settings save can be bound at a time; a second request is rejected
   by MainWindow before dispatch.
2. A callback with a non-current ID is `stale` and cannot release or mutate the
   current settings-save lifecycle.
3. A matching invalid result releases the tracker and keeps the existing
   invalid-result error projection.
4. A matching failure releases the tracker and keeps the existing error text.
5. The tracker imports only the domain `SettingsSnapshot`; it owns no Qt,
   service, theme, locale, font, editor, or notification policy.

## Alternatives considered

- **Keep the boolean and ignore callback IDs:** rejected because stale callback
  behavior remains implicit and can clear a future save if the lifecycle grows.
- **Move settings application into the tracker:** rejected because theme,
  locale, font, editor, and animation are presentation policy owned by
  MainWindow.
- **Add a general settings coordinator:** rejected as speculative; one
  operation identity is the complete current boundary.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or firmware;
MCU, BSP/HAL, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control,
and vendor-manufacturer requirements are not applicable. Public CloudWeGo
material remains transferable engineering reference only and does not
establish a private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable project references are the settings contract in
`application/settings.py`, ADR-0074/0075's Qt-free lifecycle pattern, and the
enterprise architecture migration specification.

## Verification target and limits

- A source probe proves the tracker is Qt-free, MainWindow retains settings
  application policy, and close/runner boundaries remain intact.
- A Qt-free behavior probe covers duplicate begin, stale callbacks, invalid
  results, failures, and valid completion.
- Compile, Ruff, format, handoff, package identity, and release no-go evidence
  are recorded in the handoff.
- Native Qt callback timing, runtime settings interaction, visual theme/font
  rendering, accessibility, DPI, clean-machine, cross-machine, signing,
  installer, updater, legal, support, and release-owner evidence remain unrun
  under the active no-launch/external-authorization boundary.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created.
