# ADR-0223: Refine the Workspace dock visual hierarchy

- **Status:** accepted-with-limits; D174 / UI-86 / ARCH-161 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The Workspace dock already had semantic styling, but its strong outer frame,
thick title rail, and undersized native close/float controls made the left
navigation region feel heavier and less consistent than the document rail,
editor stage, and status rail.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the
existing `WorkspaceDock` QSS:

- soften the dock frame to the normal border token;
- reduce the title emphasis rail to 2px and give the title a calmer 10px
  corner and slightly more vertical breathing room;
- align native close/float controls with the document-tab close contract at
  20px, 6px radius, and the existing hover/pressed/disabled state tokens;
- preserve the existing WorkspaceDock, workspace panel, path, status, tree,
  focus, and selection selector ownership.

Workspace navigation, file/folder activation, directory search, locale,
signals, docking policy, and application ownership remain unchanged.

## Invariants

1. Only centralized WorkspaceDock QSS rules change.
2. Close, float, dock placement, workspace loading, tree selection, file
   activation, search, locale, and error behavior remain unchanged.
3. Hover, pressed, and disabled native subcontrol states remain explicit.
4. Existing theme tokens and readable foreground derivation remain
   authoritative for every supported theme/accent endpoint.
5. Native Qt docking metrics, DPI, accessibility, and runtime appearance
   remain unclaimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material
remains an engineering reference only. No private ByteDance standard,
certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Euler the 5th / Luna max — `NO_CONCLUSION` after bounded
  waits; the window was closed without claiming a child PASS.
- Independent review window: Jason the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no independent PASS is claimed. The checkout has no Git
  baseline for complete-diff proof.
- Parent review: `PASS` for centralized selector ownership, token reuse,
  state preservation, and presentation-only scope.
- Simplification assessment: `PASS`; the existing selectors are refined
  without new state helpers, callbacks, or policy seams.

## Verification target and limits

- Authorized evidence: source inspection, WorkspaceDock QSS contract probe,
  3-theme × 4-accent Workspace contrast projection, compileall, Ruff,
  format, PyInstaller package build, package identity, project static checks,
  handoff checks, and expected release NO-GO evidence.
- The Workspace contrast projection passed with an effective minimum of 4.53
  for the Paper/Sand violet path-text pair.
- Not proven: native Qt docking/title-button painting and metrics, font
  fallback, screen-reader output, DPI, GUI/EXE launch, clean-machine and
  cross-machine behavior, signing, installer/update, legal, support-owner,
  and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `C15CBE1C3817CE754CFFCAE48C2257AD97F9514C9D5B002B73FFC3ACDF046265`
- Size: `38550852` bytes
- Source revision: `tree-sha256:34512f058e1e2c048a24b2f0c54b6f05601fb81a425e61f53410cf70182084b0`
