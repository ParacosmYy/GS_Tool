# ADR-0227: Establish message-dialog action hierarchy

- **Status:** accepted-with-limits; D178 / UI-90 / ARCH-165 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The recovery prompt already projected restore, discard, and defer through the
shared `primaryAction`, `warningAction`, and `quietAction` roles. The common
save-before-close, about, and error message boxes still left their standard
buttons on the generic button rule, so confirmation risk and dismissal intent
were not visually separated.

## Decision

Keep `MessageSurface` as the composition owner for common message boxes and
bind only the existing standard buttons to the existing visual roles:

- Save is `primaryAction`.
- Discard is `warningAction`.
- Cancel and informational/error OK actions are `quietAction`.

About and error boxes explicitly expose an OK standard button and reuse the
quiet role. The existing centralized `theme.py` selectors remain the sole
visual owner; no new palette, state machine, coordinator, or dialog policy is
introduced.

## Invariants

1. `ask_save_before_close` keeps the same Save default and the same `save`,
   `discard`, and `cancel` return mapping.
2. Error title/message localization and about localization remain unchanged.
3. No recovery decision, close policy, notification policy, locale policy, or
   application/domain boundary moves into the visual role mapping.
4. All roles reuse the canonical button tokens and readable foreground
   derivation, including the 砂金/warning path.
5. Native Qt layout, metrics, accessibility, DPI, and runtime interaction are
   not claimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only change. MCU, embedded
C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect window: Confucius the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Boyle the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for presentation-only role binding, stable return
  mapping, explicit OK actions, existing theme ownership, and PyQt6 API shape.
- Simplification assessment: `PASS`; one small helper centralizes repeated
  standard-button role assignment without introducing a new abstraction
  layer.

## Verification target and limits

- Authorized evidence: source probes for action roles and behavior
  preservation, existing theme role contract, compileall, Ruff, format,
  project checks, PyInstaller package build, artifact/manifest identity,
  handoff checks, and release-dossier invariant inspection.
- `D178-RELEASE-DOSSIER-INVARIANT-PROBE=PASS` records the expected no-go,
  ten open gates, and three exact mechanical consistency failures.
- Not proven: native Qt message-box layout/painting, button metrics,
  accessibility, DPI, GUI/EXE launch, clean-machine/cross-machine behavior,
  signing, installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `5D7946223BEC98EF8178DABAA874549320436CE279D86AD70EC636F116561C90`
- Size: `38549008` bytes
- Source revision from `dist/QuillForge.release.json`:
  `tree-sha256:48accb0eaf47a97e606323062c33bc331692a60e376e91050633292453732007`
