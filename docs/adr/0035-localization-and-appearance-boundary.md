# ADR 0035: Localization and appearance boundary

- **Status:** accepted for D9 UI-08
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The editor shell needs a Chinese/English switch, configurable fonts and sizes,
theme/accent choices, and an optional transition animation. The workspace also
needs to activate files without moving file I/O into a Qt tree widget. These
features are cross-cutting at the presentation edge but must not turn settings
storage or domain values into Qt-specific APIs.

## Decision

1. `AppearanceSettings` is an immutable domain value. It contains only bounded
   identifiers and scalar preferences: locale, theme, accent, interface font
   family/size, and motion enabled. The bounded themes are `ink-violet`,
   `paper-sand`, and the modern default `sakura-pop`; the latter uses a rose
   accent by default for a soft anime-inspired shell while preserving explicit
   existing user selections.
2. `SettingsService` owns schema versioning, schema-1 migration, defaults, and
   whitelist/range normalization. `JsonSettingsStore` owns only the versioned
   JSON mapping and atomic replacement; it imports no Qt presentation module.
3. `i18n.py` is a presentation-only bounded catalog. `MainWindow` and dialogs
   ask it for stable UI text; notifications preserve diagnostic details while
   translating known shell messages. Changing locale rebuilds menu projections
   and reprojects long-lived dialog surfaces.
4. `theme.py` owns visual tokens and application/editor projection.
   `ThemeTransitionSurface` owns one bounded opacity animation and its effect
   lifecycle; `MainWindow` decides motion enablement, target selection, and
   trigger timing. Disabling motion clears any active transition. No general
   animation framework is introduced.
5. `WorkspacePanel` converts tree items into `file_requested` or
   `directory_requested` signals. `MainWindow` rechecks workspace containment
   and delegates file reading to the existing asynchronous document service.
   The panel never reads a file or owns document state.
6. The shell's modern/cute direction is expressed through token data and
   selectors: a Sakura Pop gradient surface, larger rounded cards, pill-like
   tabs/actions, candy accent tones, and restrained `✦`/`✧` brand language.
   The editor canvas remains high-contrast and the visual treatment adds no
   domain state, animation framework, or business coupling.
7. UI-10 keeps interaction hierarchy in the same presentation boundary. One
   `on_accent` token covers saturated action foregrounds; selected, focus,
   hovered, pressed, checked, disabled, readonly, menu, popup, tab, workspace,
   input, primary, warning, and error states reuse the existing theme scale
   through explicit QSS selectors. No component owns a second styling state
   machine.

## Consequences

- New locale/theme/accent choices extend bounded catalogs and unions rather than
  adding widget-specific branches to application services.
- Settings can be loaded and normalized without creating a QApplication.
- The UI can switch language after saving without changing command IDs or plugin
  contracts.
- Native font availability, DPI, dialog metrics, and runtime visual timing are
  environment-dependent and remain explicit acceptance limits.
- The style is anime-inspired rather than a licensed character or franchise;
  the existing authored quill/forge mark remains the application icon.

## Rejected alternatives

- Qt translation files as the first contract: they would add a build/runtime
  resource pipeline before the small two-locale catalog is stable.
- A global mutable theme singleton: it would make settings order and tests of
  presentation projections implicit.
- Opening files directly from `WorkspacePanel`: it would couple a view to
  containment, document services, and asynchronous lifecycle handling.
- A cross-application animation framework: one bounded
  `ThemeTransitionSurface` is sufficient for this slice and keeps motion
  policy local to the shell.
