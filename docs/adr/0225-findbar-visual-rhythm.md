# ADR-0225: Refine the FindBar visual rhythm

- **Status:** accepted-with-limits; D176 / UI-88 / ARCH-163 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The editor Find/Replace bar already had semantic action roles and feedback
states, but the container, field labels, case-sensitivity option, and status
message did not form a consistent visual rhythm. The result was dense and
harder to scan while searching or replacing text.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the
existing FindBar QSS:

- give the bar a calmer 12px card radius and more intentional outer spacing;
- make Find/Replace labels visibly secondary and semibold;
- give the existing case-sensitivity checkbox a compact hover/focus surface;
- give the existing find-status capsule consistent semibold text and vertical
  breathing room while preserving info/working/success/warning/error states.

`FindBar` and `FindSurface` remain responsible for query/replacement values,
signals, Enter/Shift+Enter/Esc behavior, action roles, cancellation, locale,
and operation-state projection.

## Invariants

1. Only centralized FindBar QSS rules change.
2. Find, replace, replace-all, cancel, close, case-sensitive, locale,
   keyboard, loading, and status projection behavior remain unchanged.
3. Normal, hover, focus, pressed, disabled, and semantic feedback states stay
   explicit through existing selectors and tokens.
4. Existing theme tokens and readable foreground derivation remain
   authoritative for every supported theme/accent endpoint.
5. Native Qt layout metrics, font fallback, DPI, accessibility, and runtime
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

- Architect window: Noether the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Mendel the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for centralized selector ownership, token reuse,
  FindBar behavior preservation, and presentation-only scope.
- Simplification assessment: `PASS`; the existing selectors are refined
  without new state helpers, callbacks, or policy seams.

## Verification target and limits

- Authorized evidence: source inspection, FindBar QSS contract probe, FindBar
  behavior source probe, 3-theme × 4-accent contrast projection, compileall,
  Ruff, format, PyInstaller package build, package identity, project static
  checks, handoff checks, and expected release NO-GO evidence.
- The FindBar contrast projection passed with an effective minimum of 4.53 for
  the Paper/Sand violet info-status text pair.
- Not proven: native Qt FindBar layout/painting, font fallback, screen-reader
  output, DPI, GUI/EXE launch, clean-machine and cross-machine behavior,
  signing, installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `864C12C4ECD6C673428BAC801F919AD72DE9D271A384BE0CDDA8B3761786284F`
- Size: `38550917` bytes
- Source revision: `tree-sha256:2489718595018bd8f5448ee898d85e13511954fea7dc7599124d03812d59627e`
