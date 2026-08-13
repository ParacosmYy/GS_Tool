# ADR-0172: shell surface depth and primary-work-area hierarchy

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: UI-67 / ARCH-110

## Context

The shell already had centralized theme tokens and semantic selectors, but
several neighboring surfaces reused the same value: the command rail and its
hover state, the editor shell and the document canvas, and the workspace dock
and panel. That flattened the visual hierarchy even though language, fonts,
themes, accents, motion preference, focus cues, and action roles were already
available.

## Decision

Keep `src/quillforge/presentation/theme.py` as the single presentation owner
of the QSS contract and refine only the existing selectors:

- `QMainWindow#mainWindow` uses the canvas token.
- `QWidget#editorShell` uses the stage token with a restrained frame and
  radius.
- `QToolBar#commandBar` uses the stage token, a small accent edge, and keeps
  its existing hover/pressed/primary/quiet/context states.
- `QStatusBar` uses the stage token so the status rail is separate from the
  document canvas.
- The document tab widget, tab rail, and tab items use the existing surface
  ladder in a distinct outer-to-inner order.
- `WorkspaceDock` remains a framed stage and `workspacePanel` becomes the
  canvas behind its existing path/tree/feedback surfaces.

No signal, callback, widget ownership, tab/Find order, workspace activation,
locale, font, motion, document, persistence, or application policy changes.
The existing contrast-derived foreground logic remains the source of truth
for accent and warning text.

## Alternatives rejected

- Adding a new theme model or token family would duplicate the existing
  `ThemeColors` contract for a presentation-only spacing/value correction.
- Moving layout or state into `MainWindow` would widen the slice and violate
  the current presentation ownership boundary.
- Adding gradients, shadows, decorative assets, or animation would increase
  visual noise and could not be accepted under the current no-launch boundary.

## Review and evidence

Locke the 4th / Luna max was assigned the architecture assessment and returned
no conclusion within the bounded review window. Erdos the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion;
neither child PASS is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS` because the change
reuses existing tokens and selectors without adding indirection.

Authorized non-destructive evidence: `UI67-COMPILEALL=PASS`,
`UI67-RUFF=PASS`, `UI67-FORMAT=PASS`, `UI67-TOKEN-CONTRAST-PROBE=PASS`,
`UI67-SHELL-SELECTOR-PROBE=PASS`, `UI67-CHECK=PASS`,
`UI67-VERIFY-HANDOFF=PASS`, and the package identity/no-launch probe.
Runtime Qt painting, QApplication startup, native metrics, DPI, installed-font
fallback, accessibility traversal, clean-machine, and release-owner evidence
remain unrun by policy.

Public-source applicability is Python 3.12/PyQt6 presentation QSS only;
embedded C/C++ and manufacturer requirements are not applicable. Public
CloudWeGo material is an engineering reference only, not a private ByteDance
standard or compliance claim.
