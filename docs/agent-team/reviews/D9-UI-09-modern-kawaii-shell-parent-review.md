# D9 UI-09 modern Sakura Pop shell review

## Scope

This review covers the follow-up visual slice requested after UI-08: a more
modern, anime-inspired, cute visual language without changing document,
workspace, settings persistence, or plugin ownership.

## Architecture result

`PASS WITH LIMITS`.

- `ThemeId` and `SUPPORTED_THEMES` are the only application-facing extension;
  `sakura-pop` remains a bounded schema-v2 value and old `ink-violet` and
  `paper-sand` values remain valid.
- `ThemeColors` and `_ACCENT_TONES` remain presentation-owned token registries;
  the application does not import QSS or Qt types. `apply_theme()` and
  `apply_editor_palette()` remain separate projections for the shell and
  QScintilla adapter.
- SettingsDialog and i18n add only the theme option and copy needed for the
  projection. MainWindow/WorkspacePanel receive no new business state.
- The visual direction uses CSS-like gradients, rounded surfaces, pill actions,
  candy tones, and restrained `✦`/`✧` brand language. No mascot, licensed
  character, or new runtime asset pipeline was introduced; the existing
  authored quill/forge icon remains the asset boundary.

## Independent review

Luna/max Architect agent Plato (`019fe792-e12a-7041-9915-12f097751b94`)
reviewed the proposed theme extension read-only and returned `PASS WITH LIMITS`.
It specifically required synchronizing domain `ThemeId`, application
whitelists, SettingsDialog options, editor fallback validation, and the shared
presentation token registry; all were addressed. It confirmed schema v2
compatibility and the separation between application theme and editor palette.

## Simplification assessment

The smallest complete implementation is retained:

- one additional bounded theme ID rather than a mutable style engine;
- one `SAKURA_POP` token set and one accent-tone map rather than scattered
  widget-specific color literals;
- existing QSS selectors and icon loading rather than a new widget framework or
  bitmap pipeline;
- copy-only brand warmth rather than unbounded animation, mascot state, or
  plugin/theme extension points.

No behavior-preserving simplification is required beyond keeping the default
selection change limited to missing/invalid settings; explicit saved themes
remain respected.

## Public-source applicability

The embedded-enterprise workflow and embedded-code review simplifier remain not
applicable: this is Python/PyQt6 presentation code with no MCU, firmware,
BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or embedded C/C++ change. No
vendor requirement, MISRA/ISO claim, or certification claim is made.

## Authorized validation

- `uv run python -m compileall -q src/quillforge` — PASS.
- `uv run ruff check src/quillforge` — PASS.
- `uv run ruff format --check src/quillforge` — PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/package.ps1` — PASS.
- Runtime launch, QApplication creation, screenshots, installed-font/DPI/native
  style checks, and interactive visual acceptance remain unrun under policy.

