# ADR-0053: Isolate the editor-document presentation adapter

- Status: Accepted with limits
- Date: 2026-08-10
- Decision owners: Architect / fixed six-role workflow
- Delivery: D28 / ARCH-19

## Context

`MainWindow` still creates each `EditorWidget`, applies editor appearance
settings, and wires QScintilla-backed signals directly. Those operations are
presentation-adapter concerns, but they sit beside document state, save and
recovery policy, Replace All coordination, and session continuity. This makes
the coordinator harder to reason about and makes a future visual/editor
refinement touch the application coordinator unnecessarily.

## Decision

Introduce `quillforge.presentation.editor_document_surface.EditorDocumentSurface`
as the single owner of:

- creating and initially configuring an `EditorWidget` from presentation-safe
  values (`text`, `dirty`, language hint, editor settings, theme, and accent);
- refreshing the language hint after a Save As path change;
- applying an editor settings/appearance snapshot to an existing editor; and
- connecting editor modified/content/caret signals to callbacks supplied by
  `MainWindow`.

`MainWindow` remains the owner of `_DocumentTab`, document service calls,
dirty-state transitions, content versions, session writes, find/replace
semantics, recovery capture, operation locking, save/close guards, and all
application notifications. The new surface does not import an application
service, document record, persistence adapter, TaskRunner, or operation
policy.

## Invariants

1. Existing editor creation order remains: language hint, settings, text,
   then dirty state before the tab is projected.
2. Signal callbacks preserve the existing callback arguments and call order;
   the surface only routes signals and never mutates document state.
3. Applying settings to existing editors uses the same editor adapter methods
   and the same current theme/accent values.
4. The surface has no Qt application singleton, global event bus, or second
   editor/document state model.
5. The change is source-level and non-destructive under the permanent no-launch
   policy; Qt startup, editor interaction, visual rendering, and accessibility
   remain explicit limits.

## Alternatives rejected

- Moving `_DocumentTab` or save/recovery state into the surface would mix the
  application/document boundary with widget construction.
- Adding a generic UI factory or dependency-injection framework would broaden
  the migration beyond this concrete, observable seam.
- Leaving signal wiring in `MainWindow` would preserve the direct
  QScintilla coupling this slice is intended to remove.

## Public-source applicability

This is Python/PyQt6 desktop code, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are N/A for
MCU/vendor constraints. CloudWeGo public material remains an engineering
reference for explicit boundaries only; it is not a private ByteDance standard
or certification evidence.

## Verification target

The parent will run compile, Ruff, format, handoff, project checks, a static
boundary probe, package identity checks, and the existing release verifier.
Runtime Qt/editor interaction and unit-test assets remain unrun under project
policy.
