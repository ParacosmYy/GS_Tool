# ADR-0099: Settings preview surface boundary

- **Status:** accepted-with-limits; D74 / UI-47 bounded slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D73 added a live appearance preview, but the Settings dialog still contained
the preview's complete widget tree, layout, style projection, and copy. That
made a form coordinator responsible for two presentation concerns and made a
future preview refinement harder to isolate.

## Decision

Extract `SettingsPreviewSurface` into
`src/quillforge/presentation/settings_preview.py`. It owns the preview
`QFrame`, child labels/layouts, accessibility name, localized preview copy,
and token/style projection. Its single `project(...)` method receives the
current pending selection and display labels from `SettingsDialog` and has no
side effects beyond its own widgets.

`SettingsDialog` remains the owner of editable controls, locale selection,
`SettingsSnapshot` construction, Save/Cancel, and the persistence boundary. It
only composes the surface and projects current control values into it. The
canonical token resolver and stylesheet remain in `theme.py`.

## Invariants

1. The new surface does not import application services, persistence, domain
   policy, filesystem, subprocess, or MainWindow code.
2. `project(...)` is one-way presentation projection; it cannot apply a theme,
   save settings, or mutate the dialog's source controls.
3. There is no second settings model: the dialog controls remain the source of
   pending values, while the surface owns only rendered Qt objects.
4. Existing locale, Save/Cancel, snapshot, global theme timing, editor,
   motion, and notification behavior remain source-equivalent.
5. Static boundary and package evidence do not claim native Qt runtime proof.

## Alternatives considered

- **Leave all preview code in `SettingsDialog`:** rejected; it preserves a
  mixed form/visual responsibility and increases future change coupling.
- **Create a shared settings view-model:** rejected; the current surface needs
  no business state or cross-widget synchronization beyond one projection call.
- **Move preview token resolution into application code:** rejected; QSS and
  visual tokens are presentation concerns and would violate dependency flow.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop presentation code. Embedded C/C++, MCU, BSP/HAL,
CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and
manufacturer requirements are not applicable. Public CloudWeGo material
remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Verification target and limits

- `D74-PRESENTATION-BOUNDARY-PROBE=PASS` verifies the surface contract and
  prevents the preview dependency from remaining in `SettingsDialog` or
  leaking into application modules.
- Compileall, Ruff, format, package identity, handoff, and repository checks
  are recorded in the D74 handoff.
- The Architect and independent review windows returned no conclusion; no
  child PASS is claimed.
- Native Qt startup/rendering, modal interaction, accessibility, DPI,
  installed-font behavior, clean-machine, cross-machine, signing, legal, and
  release-owner evidence remain unrun or open.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created.
