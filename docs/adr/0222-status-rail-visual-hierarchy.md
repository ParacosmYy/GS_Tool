# ADR-0222: Refine the status-rail visual hierarchy

- **Status:** accepted-with-limits; D173 / UI-85 / ARCH-160 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The bottom status area used a strong full-width boundary while the transient
message and permanent shell-status rail were close to the surrounding surface.
The result was visually flat in normal states and overly heavy at the shell
edge.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the existing
status selectors:

- soften the outer `QStatusBar` into the shell surface;
- give transient messages and the permanent status rail a distinct secondary
  surface and consistent pill radius;
- increase the phase pill breathing room while retaining explicit ready,
  working, attention, and error states;
- keep context separation and all info/success/warning/error feedback colors
  token-bound.

`StatusSurface`, `StatusRail`, notification timers, localization, phase
precedence, and application policy remain unchanged.

## Invariants

1. Only centralized status QSS rules change.
2. Message text, locale, tooltip, visibility, timer, severity, phase labels,
   accessible names, and state properties remain unchanged.
3. All four message and four phase state selectors remain explicit.
4. Existing readable foreground derivation remains authoritative for every
   theme/accent endpoint.
5. Native Qt rendering, DPI, accessibility, and runtime appearance remain
   unclaimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Chandrasekhar the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Poincare the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for status-selector coverage, palette-token reuse,
  notification/phase behavior preservation, and presentation-only ownership.
- Simplification assessment: `PASS`; existing status selectors are refined
  without new state helpers, callbacks, or policy seams.

## Verification target and limits

- Authorized evidence: source inspection, QSS contract probe, 3-theme ×
  4-accent message/phase contrast projection, compileall, Ruff, format,
  PyInstaller package build, package identity, project static checks, handoff
  checks, and expected release NO-GO evidence.
- Not proven: native Qt status-bar painting/layout, font metrics, screen-reader
  output, DPI, GUI/EXE launch, clean-machine/cross-machine behavior, signing,
  installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `315FD62225BA2D3402EAB8697C870BA6C514BC0D5FDA020230180E4B153FA602`
- Size: `38551243` bytes
- Source revision: `tree-sha256:19dd315c12b2af54791344fa571be59df59187dbd6998caa968d21a83154f0c9`
