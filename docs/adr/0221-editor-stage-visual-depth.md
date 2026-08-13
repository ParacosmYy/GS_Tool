# ADR-0221: Refine editor-stage visual depth

- **Status:** accepted-with-limits; D172 / UI-84 / ARCH-159 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The central editor shell used a strong outer frame around a separately framed
QScintilla canvas. That double boundary added visual weight to the largest
surface in the application and weakened the calmer command and tab hierarchy.

## Decision

Keep `presentation.theme` as the sole visual owner and refine only
`QWidget#editorShell` and `QsciScintilla#editor` styling:

- use the shell as a softer stage surface with a restrained border;
- give the editor canvas the primary readable surface and a slightly larger
  radius;
- preserve the existing focus accent, selection background, selection text,
  and editor palette projection.

`EditorWidget` and `EditorShellSurface` retain all font, lexer, selection,
editing, document, and signal ownership. No editor behavior or application
policy changes.

## Invariants

1. Only centralized editor-shell/editor QSS rules change.
2. Font, lexer, syntax colors, caret, selection, line numbers, wrapping,
   document state, signals, and operation policy remain unchanged.
3. Focus and selection selectors remain explicit and token-bound.
4. No hard-coded accent endpoint or new presentation boundary is introduced.
5. Native QScintilla rendering, DPI, accessibility, and runtime appearance are
   not claimed without authorized execution.

## Public-source applicability and embedded gate

This is a Python 3.12/PyQt6 desktop presentation-only stylesheet change. MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM,
power-control, motor-control, and manufacturer requirements are not applicable.
The mandatory embedded assurance workflow and simplifier are N/A for this
source scope; no embedded source was changed. Public CloudWeGo material remains
an engineering reference only. No private ByteDance standard, certification,
MISRA, ISO 26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect window: Peirce the 5th / Luna max — `NO_CONCLUSION` after bounded
  waits; the window was closed without claiming a child PASS.
- Independent review window: Galileo the 5th / Luna max — `NO_CONCLUSION`
  after bounded waits; no independent PASS is claimed. The checkout has no
  Git baseline for complete-diff proof.
- Parent review: `PASS` for theme-only ownership, focus/selection preservation,
  palette reuse, and editor behavior preservation.
- Simplification assessment: `PASS`; existing selectors are refined without
  new adapters, callbacks, delegates, or theme tokens.

## Verification target and limits

- Authorized evidence: source inspection, QSS contract probe, 3-theme ×
  4-accent editor/stage/focus/selection contrast projection, compileall, Ruff,
  format, PyInstaller package build, package identity, project static checks,
  handoff checks, and expected release NO-GO evidence.
- Not proven: native QScintilla painting/layout, actual font metrics,
  screen-reader output, DPI, GUI/EXE launch, clean-machine/cross-machine
  behavior, signing, installer/update, legal, support-owner, and release-owner
  gates.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run. No GUI/EXE launch was performed under the active no-launch policy.

## Artifact identity

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `BEB4E9626AE27F7FB3DBDDFE3985D4F3D559F6760B073D3C8ED1887C3A292647`
- Size: `38551851` bytes
- Source revision: `tree-sha256:840ab21f827dbf7574dfd7b3bee234c63a897436e645699d123659e7a8be300e`
