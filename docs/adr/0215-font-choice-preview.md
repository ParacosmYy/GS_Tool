# ADR-0215: Font-choice preview in Settings

- **Status:** accepted-with-limits; D166 / UI-78 / ARCH-153 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Settings dialog already allowed selecting the supported interface and
editor font families, but both controls rendered every option as plain text.
That made the font-style choice difficult to judge before saving, especially
when the user was also adjusting interface or editor size.

## Decision

Keep the existing `QComboBox` controls and the existing application-owned
font allowlists. After the allowlisted items are added, assign each item a
`QFont(family)` through `Qt.ItemDataRole.FontRole`. The popup therefore
previews each choice in its own family without changing the selected value or
the settings persistence contract.

## Invariants

1. Only `src/quillforge/presentation/settings_dialog.py` changes for this
   slice.
2. `SUPPORTED_UI_FONTS` and `SUPPORTED_EDITOR_FONTS` remain the sole item
   sources; order and display text are unchanged.
3. `UserRole` data remains the existing theme/accent/language contract;
   `settings_snapshot()` continues to read font choices through
   `currentText()`.
4. `SettingsSnapshot`, `SettingsService`, persistence, theme application,
   editor application, locale refresh, size ranges, and motion behavior are
   unchanged.
5. The helper remains Qt-only presentation code. No font policy, fallback
   resolver, service, singleton, or second settings path is introduced.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation change. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect: Epicurus the 5th / Luna max — `PASS` with the bounded condition
  that only the presentation file changes and the existing value contract is
  preserved.
- Independent review: Singer the 5th / Luna max — `NO_CONCLUSION`; the
  reviewer confirmed the code/API boundary but could not prove the complete
  diff because this checkout has no Git baseline.
- Parent review: `PASS` for imports, FontRole usage, allowlist ownership,
  `currentText()` compatibility, and scope.
- Simplification assessment: `PASS`; one private helper removes duplication
  between the two font controls without adding a new abstraction layer.

## Verification target and limits

- Authorized evidence: AST/source-shape probe, Ruff, format check,
  PyInstaller package build, package identity, handoff/register/index
  synchronization, static project check, and expected release NO-GO evidence.
- Not proven: native popup rendering, installed-font fallback metrics, DPI,
  accessibility, clean-machine behavior, cross-machine behavior, signing,
  installer/update, legal, support, or release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active project policy.

