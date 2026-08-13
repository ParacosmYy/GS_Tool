# ADR-0316: Startup settings preflight

Status: accepted with limits  
Date: 2026-08-12  
Delivery: D280 / ARCH-250

## Context

QuillForge has a no-window startup diagnostic for isolating failures that occur
before a native window can be inspected. The settings service already treats a
missing, unreadable, or invalid local JSON snapshot as a safe-default case, but
the diagnostic did not report whether the user-local settings file existed or
could be decoded. That made a configuration-related startup report too coarse.

## Decision

Add a `settings_preflight` probe to `quillforge.app` and keep it on the
Qt-free diagnostic path. The probe:

1. resolves the existing `default_settings_path()`;
2. reads through the existing `JsonSettingsStore` decoder once;
3. applies the existing `normalize_settings` function, or the existing
   `DEFAULT_SETTINGS` fallback when no snapshot is decoded; and
4. reports only the path, file presence, decode validity, and normalized schema
   version.

The probe has no write path and does not include locale, theme, accent, font,
motion, or other preference values. The outer diagnostic probe boundary still
converts unexpected exceptions into a failed check instead of allowing the
reporter itself to abort.

## Consequences

The report can distinguish a missing/invalid settings file from a valid legacy
snapshot that was normalized to the current schema. Startup behavior remains
owned by `SettingsService`; the diagnostic reuses its store and normalizer
instead of creating a second validation policy. A native launch is still
required to prove the full Windows startup path.

## Public-source applicability

- Python 3.12 `pathlib.Path` resolution and file predicates are applicable
  standard-library references: <https://docs.python.org/3/library/pathlib.html>.
- Python 3.12 JSON decoding behavior is applicable to the existing local store:
  <https://docs.python.org/3/library/json.html>.
- No manufacturer requirement is applicable. This is not embedded C/C++,
  MCU, BSP/HAL, RTOS, ISR/DMA, driver, bootloader, or firmware work; no
  MISRA, ISO 26262, ASPICE, or certification claim is made.

## Verification boundary

Source startup diagnostic, compileall, Ruff, formatting, project checks,
PyInstaller PE/archive inspection, and package identity were authorized and
passed. EXE/Qt launch, native rendering, clean-machine startup, signing,
installer, updater, registry, and release-go gates remain unrun or open.
