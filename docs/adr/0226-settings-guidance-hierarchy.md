# ADR-0226: Refine the Settings guidance hierarchy

- **Status:** accepted-with-limits; D177 / UI-89 / ARCH-164 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

Settings already exposed language, theme, accent, font, size, motion, editor
options, and a live preview. The two important guidance lines at the bottom of
the dialog were still plain text, so save timing and font fallback information
did not belong visually to the same card system as the controls and preview.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the
existing Settings QSS:

- render the existing apply-after-save note as a compact information capsule
  with the alternate accent edge;
- render the existing font-fallback note as a quieter supporting capsule with
  the pink accent edge;
- reuse the existing surface, border, radius, text, and spacing tokens.

`SettingsDialog`, `SettingsPreviewSurface`, and the settings application
boundary remain responsible for locale, preview, `SettingsSnapshot`, font
fallback messaging, Save/Cancel, persistence, and motion policy.

## Invariants

1. Only centralized Settings note QSS rules change.
2. Note text, locale refresh, preview updates, font fallback meaning,
   SettingsSnapshot values, Save/Cancel, persistence, and motion behavior
   remain unchanged.
3. Both notes remain readable under all supported theme/accent combinations.
4. Existing theme tokens and readable foreground derivation remain
   authoritative for every supported theme/accent endpoint.
5. Native Qt dialog layout, font fallback, DPI, accessibility, and runtime
   appearance remain unclaimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material
remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Wegener the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Nash the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no independent PASS is claimed. The checkout has no Git
  baseline for complete-diff proof.
- Parent review: `PASS` for stable object-name ownership, token reuse,
  Settings behavior preservation, and presentation-only scope.
- Simplification assessment: `PASS`; the existing note selectors are refined
  without new state helpers, callbacks, or policy seams.

## Verification target and limits

- Authorized evidence: source inspection, Settings note QSS contract probe,
  Settings behavior source probe, 3-theme × 4-accent note contrast
  projection, compileall, Ruff, format, PyInstaller package build, package
  identity, project static checks, handoff checks, and expected release NO-GO
  evidence.
- The Settings note contrast projection passed with an effective minimum of
  4.87 for the Paper/Sand violet apply-note text pair.
- Not proven: native Qt dialog layout/painting, font fallback, screen-reader
  output, DPI, GUI/EXE launch, clean-machine and cross-machine behavior,
  signing, installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `EBC815177492FFC9E5ABD5551C428BBDC73D71C9DE72B5F8CB278CBB0814842D`
- Size: `38550594` bytes
- Source revision: `tree-sha256:27df2957ea16ed501da5009efb05c49996837d830ad22b3c5f6b9316b6dd740a`
