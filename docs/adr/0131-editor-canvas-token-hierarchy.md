# ADR-0131: Editor canvas token hierarchy

- **Status:** accepted-with-limits; D104/UI-53 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shell already resolved surface, accent, selection, and feedback colors in
`presentation.theme`, but `apply_editor_palette()` still mixed those tokens
with editor-local raw syntax colors. The editor therefore had no single
explicit contract for its canvas, gutter, selection foreground, caret,
current-line highlight, or syntax roles. In particular, the light paper theme
used string/operator colors that were not consistently readable as normal
text.

## Decision

Add the frozen `EditorColorTokens` value object and the
`editor_color_tokens(theme_id, accent_id)` resolver beside the existing shell
theme resolver. The resolver owns the editor's semantic color roles and uses a
4.5:1 normal-text contrast guard with a readable fallback where an accent
endpoint is too light on the canvas. `apply_editor_palette()` remains the one
adapter projection: it applies palette, selection, gutter, caret, current-line,
matched-brace, and Python lexer colors through optional QScintilla methods.

The editor QSS adds only the existing `editor` object-name surface and focus
cue. `EditorWidget` remains responsible for QScintilla construction and
settings application; MainWindow, editor operations, language policy,
document state, persistence, and close behavior remain unchanged.

## Invariants

1. Unknown theme/accent identifiers continue to use the existing safe theme
   fallback; the new resolver derives from that same resolved token set.
2. Selection foreground is resolved against the actual selection background;
   syntax foregrounds are resolved against the editor canvas; gutter text is
   resolved against the gutter surface.
3. The editor adapter does not expose QScintilla types to application/domain
   code, and no Qt, filesystem, persistence, or operation state enters the
   token value object.
4. Existing `EditorWidget` methods, lexer selection, text capture, wrapping,
   font settings, cursor state, and operation callbacks are unchanged.
5. Optional adapter methods remain guarded so the presentation contract stays
   compatible with the existing editor adapter surface.

## Alternatives considered

- **Keep raw syntax colors in `apply_editor_palette()`:** rejected; it creates
  a second visual source of truth and caused light-theme contrast drift.
- **Add every editor role to `ThemeColors`:** rejected; shell tokens and
  editor-specific syntax roles would become one oversized record with weaker
  ownership clarity.
- **Use QSS only:** rejected; QScintilla lexer and margin APIs are the actual
  projection points for code text, so QSS alone cannot establish the syntax
  hierarchy.
- **Move editor styling into MainWindow:** rejected; it would reverse the
  presentation boundary and couple composition policy to visual details.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6/QScintilla desktop presentation code. No MCU,
embedded C/C++, BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot, Flash/NVM,
power-control, motor-control, or manufacturer requirement applies. The
mandatory embedded assurance gate is therefore `N/A`. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Review and simplification

- Architect: Dirac the 3rd / Luna max; the bounded read-only wait timed out and
  the agent was closed. Status is `NO_CONCLUSION`; no child architecture PASS
  is claimed.
- Independent review: Linnaeus the 3rd / Luna max; the bounded read-only wait
  timed out and the agent was closed. Status is `NO_CONCLUSION`; no child
  independent PASS is claimed.
- Parent source review: PASS for token ownership, fallback behavior, adapter
  call signatures, dependency direction, and preservation of editor policy.
- Simplification assessment: PASS. A small dedicated editor token record is
  simpler than widening the shell token record or adding a second styling
  service; no further safe reduction was identified.

## Verification target and limits

- Required: source/contract and all-theme contrast probes, compileall, Ruff,
  format, package identity, handoff/register/index synchronization, and the
  expected release NO-GO dossier.
- Not proven by this slice: native QSS specificity, QScintilla rendering,
  screen-reader output, DPI/font fallback, screenshots, startup, worker
  timing, clean-machine behavior, signing, installer, legal, or release-owner
  evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created
  or run under the active project policy.
