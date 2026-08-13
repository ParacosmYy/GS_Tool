# ADR-0224: Refine the Command Palette visual hierarchy

- **Status:** accepted-with-limits; D175 / UI-87 / ARCH-162 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Command Palette already had a themed query field and selected result row,
but its result list had no dedicated hover or focus boundary. The keyboard
hint was also rendered as low-emphasis text, so mouse and keyboard scanning
were harder than the command workflow required.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the
existing Command Palette QSS:

- add a token-driven focus boundary to the result list;
- add a readable hover state and an explicit selected-hover state without
  changing the selected-row meaning;
- promote the existing keyboard hint into a compact secondary-surface capsule
  with a restrained accent edge;
- preserve the query field, result population, current-row selection, item
  activation, return-key acceptance, and localized text ownership.

`CommandPaletteDialog` and `CommandPaletteSurface` remain responsible for
command filtering, stable IDs, selection, execution handoff, locale, and
modal lifecycle.

## Invariants

1. Only centralized Command Palette QSS rules change.
2. Query filtering, result ordering, current-row selection, item activation,
   return-key acceptance, command IDs, locale, and dialog lifecycle remain
   unchanged.
3. Normal, hover, selected, selected-hover, focus, and disabled behavior
   remain distinguishable through existing semantic surfaces and borders.
4. Existing theme tokens and readable foreground derivation remain
   authoritative for every supported theme/accent endpoint.
5. Native Qt list painting, focus metrics, DPI, accessibility, and runtime
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

- Architect window: Herschel the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Euclid the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for centralized selector ownership, token reuse,
  command behavior preservation, and presentation-only scope.
- Simplification assessment: `PASS`; the existing selectors are refined
  without new state helpers, callbacks, or policy seams.

## Verification target and limits

- Authorized evidence: source inspection, Command Palette QSS contract probe,
  command behavior source probe, 3-theme × 4-accent contrast projection,
  compileall, Ruff, format, PyInstaller package build, package identity,
  project static checks, handoff checks, and expected release NO-GO evidence.
- The Command Palette contrast projection passed with an effective minimum of
  4.87 for the Paper/Sand violet hint text pair.
- Not proven: native Qt list painting/focus/layout, font fallback,
  screen-reader output, DPI, GUI/EXE launch, clean-machine and cross-machine
  behavior, signing, installer/update, legal, support-owner, and release-owner
  gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `93A85B6A7DD303AD53F89EBC5B71C66198B586F7AF7BBE2294687B36415519E2`
- Size: `38549974` bytes
- Source revision: `tree-sha256:3ce23c2c9031e1c7bf23889f2f07e6bd3cd7b3d15bf71f1f51e3bd753a28b1d7`
