# ADR-0220: Flatten the document-tab visual hierarchy

- **Status:** accepted-with-limits; D171 / UI-83 / ARCH-158 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The document tab rail had a framed outer container and individually filled,
bordered inactive tabs. That repeated box treatment made the editor shell feel
dense and visually dated, especially beside the lighter D170 command rail.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only the
`QTabBar#documentTabBar` projection:

- keep one restrained rail container;
- make inactive tabs transparent until hover or selection;
- retain a raised selected surface with accent edge/bottom emphasis;
- give focus its own readable surface and border;
- keep disabled and close-button states explicit, with a consistent close
  target size and radius.

`DocumentTabSurface` remains responsible for tab creation/removal, title and
modified-icon projection, current-index signals, and document identity. No tab
behavior, text, icon provider, close policy, or application strategy changes.

## Invariants

1. Only the centralized document-tab QSS rules change for this slice.
2. Existing tab creation, removal, selection, title, modified state, close
   signal, icon refresh, and keyboard/focus behavior remain source-equivalent.
3. Generic tab selectors and document-specific selectors retain explicit
   hover, selected, focus, disabled, and close-button coverage.
4. Foreground and accent colors continue to use the existing theme tokens;
   no hard-coded endpoint is introduced.
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

- Architect window: McClintock the 5th / Luna max — `NO_CONCLUSION` after
  bounded waits; the window was closed without claiming a child PASS.
- Independent review window: Descartes the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for presentation-only ownership, selector coverage,
  contrast-token reuse, and tab behavior preservation.
- Simplification assessment: `PASS`; the existing QSS seam is reused without
  new widget helpers, callbacks, delegates, or styling layers.

## Verification target and limits

- Authorized evidence: source inspection, QSS contract probe, 3-theme ×
  4-accent tab-state contrast projection, compileall, Ruff, format, PyInstaller
  package build, package identity, project static checks, handoff checks, and
  expected release NO-GO evidence.
- Not proven: native Qt tab painting/layout, actual font metrics, screen-reader
  output, DPI, GUI/EXE launch, clean-machine behavior, cross-machine behavior,
  signing, installer/update, legal, support-owner, and release-owner gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `862C7FE18EDC82E73F3CD5A9768566D41EEEA313FD1F89B3BF58F3759865B242`
- Size: `38550744` bytes
- Source revision: `tree-sha256:0c2b2ca38eaf6f596c356aac5790487d5afe610d3f94dd827e08876cb0b35363`
