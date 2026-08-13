# ADR 0005: Versioned settings and ID-based command discovery

- **Status:** accepted for D5/D9 UI-08
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The command registry already provides stable IDs for host and trusted plugin commands, but menu projection is not sufficient for keyboard-first discovery. The editor also needs a small persistent preference contract without making Qt controls, plugin state, or malformed user files part of the application API.

## Decision

1. `CommandPaletteDialog` renders the current `CommandRegistry` and stores only the selected `command_id`. `MainWindow` resolves that ID against the live registry immediately before execution; a command unregistered while the palette is open is rejected and the menu projection is refreshed.
2. `SettingsSnapshot` is schema-versioned. Schema 2 persists the editor font family, size, wrapping, and line-number preference plus an immutable `AppearanceSettings` value containing locale, theme, accent, interface font/size, and motion preference. `SettingsService` supplies defaults, migrates the supported schema 1 editor subset, and normalizes unsupported values to bounded defaults. The settings adapter writes a small JSON envelope atomically in the per-user app directory.
3. Settings writes run through the existing `TaskRunner`. The UI applies preferences only after the worker reports a valid persisted snapshot, and close is held while that write is in flight.
4. `EditorEngine` exposes semantic font-family, font-size, line-wrap, line-number, and theme operations. QScintilla enums and fonts remain inside `EditorWidget`; plugins do not receive settings widgets or private storage paths.
5. The presentation layer owns a bounded translation catalog, theme token projection, and optional fade transition. The application composition root loads the initial snapshot once, applies the application theme, and injects the same snapshot into `MainWindow`; subsequent saves update the persisted snapshot before re-projecting it.

## Rejected alternatives

- Caching `Command` objects in the palette: plugin lifecycle changes could leave stale executable callbacks.
- `QSettings` as the application contract: it would hide schema/version behavior and make migration evidence less explicit.
- Persisting arbitrary plugin settings in D5: governed extension permissions and compatibility belong to D6.
- Applying settings before persistence succeeds: a disk failure would make the visible session diverge from the durable contract.

## Limits and follow-up

Settings are local, unsynchronized, and unencrypted. The catalog is intentionally limited to English and Simplified Chinese, two theme surfaces, four accent colors, and whitelisted fonts; plugin-specific settings remain outside this contract. Runtime font availability, DPI, native dialog metrics, and visual acceptance remain environment-owned. D6 owns plugin settings governance and D8 owns enterprise configuration/release policy.
