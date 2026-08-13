# ADR-0228: Give plugin-catalog guidance a supporting capsule

- **Status:** accepted-with-limits; D179 / UI-91 / ARCH-166 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The plugin catalog dialog already separated its summary, entry list, and
governance action rail, but its localized governance hint remained plain muted
text. That left the most important operator guidance visually weaker than the
surrounding surfaces.

## Decision

Keep `presentation.theme` as the sole visual owner and scope a supporting
capsule to `QDialog#pluginCatalogDialog QLabel#dialogHint`:

- use the existing `surface_2`, `border`, `text_secondary`, radius, and spacing
  tokens;
- use the pink accent edge already associated with supporting/editor guidance;
- leave `PluginCatalogDialog` responsible for hint text, locale refresh,
  wrapping, list selection, governance signals, and trust/approval policy.

No plugin state, dynamic property, callback, coordinator, or new palette token
is introduced.

## Invariants

1. The hint keeps its `dialogHint` object name, localized text, and word-wrap.
2. Summary/list/action surfaces and plugin governance behavior are unchanged.
3. The selector is scoped to the catalog dialog and does not restyle the
   plugin-status dialog by accident.
4. The supporting text remains readable across all 3 themes × 4 accents; the
   accent edge is not used as the only state signal.
5. Native Qt layout, metrics, accessibility, DPI, and runtime interaction are
   not claimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not
applicable. The mandatory embedded assurance workflow and simplifier are N/A
for this source scope; no embedded source was changed. Public CloudWeGo
material remains an engineering reference only. No private ByteDance
standard, certification, MISRA, ISO 26262, ASIL, ASPICE, or compliance claim
is made.

## Review and simplification

- Architect window: Cicero the 5th / Luna max — `NO_CONCLUSION` after bounded
  waits; the window was closed without claiming a child PASS.
- Independent review window: Pauli the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; no independent PASS is claimed. The checkout has no Git
  baseline for complete-diff proof.
- Parent review: `PASS` for selector scope, token reuse, hint ownership, and
  behavior preservation.
- Simplification assessment: `PASS`; the existing generic `dialogHint` rule
  remains intact while one dialog-specific rule supplies the missing hierarchy,
  without a new visual abstraction.

## Verification target and limits

- Authorized evidence: scoped QSS contract, PluginCatalogDialog behavior
  source probe, 3-theme × 4-accent contrast projection, compileall, Ruff,
  format, project checks, PyInstaller package build, artifact/manifest
  identity, handoff checks, and expected release no-go evidence.
- `D179-PLUGIN-HINT-CONTRAST-PROBE=PASS` passed with a minimum ratio of 4.87
  for the Paper/Sand violet endpoint.
- Not proven: native Qt dialog layout/painting, font metrics, accessibility,
  DPI, GUI/EXE launch, clean-machine/cross-machine behavior, signing,
  installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `91628A72591EBC6D7DDAC24742616A9A8972598CCC895AB7E9957523DDE84167`
- Size: `38549076` bytes
- Source revision from `dist/QuillForge.release.json`:
  `tree-sha256:c10a99ac4ac7ea3385ab0266a05fc3f8d70ca7df82d0ba84987c49c29137cd68`
