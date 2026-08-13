# ADR-0229: Font-style settings contract and projection

## Status

Accepted with limits for D180 / UI-92 / ARCH-167.

## Context

Settings already persisted locale, theme, accent, font family, and font size,
but it did not expose a font-style choice. The user-visible gap was that the
interface and editor could not select regular, semibold, bold, or italic
style, and the preview could not prove which style was pending before Save.

The change crosses the settings model, versioned normalization, JSON storage,
settings dialog, preview, generated QSS, and the QScintilla adapter. The
boundary must remain small and must not make the domain depend on Qt.

## Decision

1. Define one Qt-free `FontStyle` literal contract with the supported values
   `regular`, `semibold`, `bold`, and `italic`.
2. Increment the settings schema from v2 to v3. Normalize v1/v2 payloads by
   defaulting absent style fields to `regular`; malformed values fall back to
   the existing validated defaults.
3. Encode and decode both `editor.font_style` and
   `appearance.ui_font_style` in the existing atomic JSON settings store.
4. Add two settings controls with stable object names, localized labels, and
   localized option text. The existing one-way preview now projects both
   interface and editor style through copied `QFont` values.
5. Keep Qt-specific weight/italic mapping in
   `presentation.font_style`. The application and domain layers consume only
   the shared literal contract.
6. Apply the editor style through `EditorWidget.set_font_style`, preserving
   the current family and size, and apply interface style through the existing
   theme stylesheet. The established save projection order remains unchanged.

## Ownership and non-goals

- `domain.models` owns the value contract only.
- `application.settings` owns schema/version validation only.
- `infrastructure.settings_store` owns JSON compatibility only.
- `SettingsDialog` and `SettingsPreviewSurface` own controls and pending
  presentation only.
- `presentation.font_style`, `theme.py`, and `EditorWidget` own Qt projection.
- No editor operation, lexer choice, theme token, plugin contract, persistence
  location, Save/Cancel decision, or application policy changes.
- Native font fallback, font metrics, DPI, accessibility, and live Qt visual
  acceptance are intentionally outside this source/offscreen slice.

## Public-source applicability

This is a Python 3.12/PyQt6 desktop change. Python `Literal` typing and the Qt
font/event-loop boundary are engineering references already recorded in the
enterprise architecture specification; they are not manufacturer requirements.
Public CloudWeGo material remains a transferable engineering reference only.
No private ByteDance standard, MISRA, ISO 26262, ASPICE, ASIL, or certification
claim is made. Embedded C/C++, MCU, RTOS, BSP/HAL/CMSIS, ISR/DMA, boot/OTA,
Flash/NVM, power, and motor-control requirements are not applicable.

## Review and verification

- Architect window: Turing the 5th / Terra max — `NO_CONCLUSION` after the
  bounded wait; no child PASS is claimed.
- Independent review window: Mencius the 5th / Terra max — `NO_CONCLUSION`
  after the bounded wait; no child PASS is claimed.
- Parent review: `PASS`.
- Simplification assessment: `PASS`; the existing settings projection
  coordinator and editor adapter boundary were reused, with one focused Qt
  projection helper and no unrelated refactor.
- Authorized non-destructive evidence: settings contract/migration JSON
  probe, bilingual i18n placeholder probe, 3-theme × 4-accent × 4-style QSS
  probe, QFont projection probe, source wiring probe, `compileall`, Ruff,
  project check, packaging, manifest identity, handoff verification, and the
  expected release no-go dossier.
- GUI/QApplication startup, screenshots, native rendering, clean-machine,
  cross-machine, and release-owner evidence remain unrun under project policy.
