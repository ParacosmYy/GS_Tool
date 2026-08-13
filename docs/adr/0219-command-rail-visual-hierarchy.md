# ADR-0219: Refine the command-rail visual hierarchy

- **Status:** accepted-with-limits; D170 / UI-82 / ARCH-157 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The command rail was already centralized, but its strong outer border, heavy
accent rule, compact controls, and uniformly outlined presentation made the
shell feel visually dense. D170 targets the first-read shell surface without
changing command behavior or spreading style decisions into command wiring.

## Decision

Keep `presentation.theme` as the single visual owner and refine only the
`QToolBar#commandBar` and `QLabel#toolbarContext` QSS projection:

- use a lighter card surface with a restrained two-pixel brand edge;
- increase card breathing room, button hit height, and corner consistency;
- keep normal, hover, focus, pressed, checked, disabled, primary, quiet, and
  context states explicit;
- make quiet actions less prominent and keep the context label as a compact
  visual anchor.

`CommandSurface` remains responsible for menu/toolbar construction, command
labels, shortcuts, icons, callbacks, and locale refresh. No signal, command
registry, application policy, theme token, or domain boundary changes.

## Invariants

1. Only the centralized stylesheet changes for the visual slice.
2. Existing command order, labels, shortcuts, callbacks, icon refresh, and
   toolbar role properties remain unchanged.
3. All command-role and interaction-state selectors remain present.
4. The existing palette-derived foreground tokens continue to own contrast;
   no hard-coded foreground is introduced for the command rail.
5. No native rendering, DPI, accessibility, or runtime behavior is claimed
   from source-only evidence.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Raman the 5th / Luna max — `NO_CONCLUSION` after bounded
  waits; the window was closed without claiming a child PASS.
- Independent review window: Copernicus the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for presentation-only ownership, state coverage,
  contrast-token reuse, and behavior preservation.
- Simplification assessment: `PASS`; the existing centralized QSS seam is
  reused without new helpers, callbacks, selectors outside the current rail,
  or role duplication.

## Verification target and limits

- Authorized evidence: source inspection, QSS contract probe, 3-theme ×
  4-accent state contrast projection, compileall, Ruff, format, PyInstaller
  package build, package identity, project static checks, handoff checks, and
  expected release NO-GO evidence.
- Not proven: native Qt layout/painting, actual font metrics, screen-reader
  output, DPI, GUI/EXE launch, clean-machine behavior, cross-machine behavior,
  signing, installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `09608EA39AA7C6278AD70F98E90E29C6577EB70D2D88EE30CA73BB6DC80262EE`
- Size: `38550855` bytes
- Source revision: `tree-sha256:71d3c175d90225f7045d1a9af105c2395f0f9e23eeb03108b447b9cbfd937fdc`
