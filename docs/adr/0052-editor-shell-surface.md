# ADR-0052: Extract the central editor shell surface

- **Status:** accepted-with-limits; D27 / ARCH-18 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly created the `editorShell` QWidget and QVBoxLayout, then
assembled the document-tab and FindBar surfaces into that layout. The shell
coordinator should own the central presentation composition while MainWindow
continues to own document records, editor operations, close/save/recovery
policy, and all semantic callbacks.

## Decision

Add `quillforge.presentation.editor_shell_surface.EditorShellSurface`. The
surface owns the central QWidget, zero-margin vertical layout,
`DocumentTabSurface`, `FindSurface`, their parentage, initial FindBar hidden
state, and the locale route for the FindBar. It exposes only the central
widget and the existing semantic `tabs`/`find` surfaces needed by MainWindow.

`MainWindow` retains all callback implementations, document/editor state,
operation policy, tab lifecycle, and command behavior. It only constructs the
composition surface and installs its central widget.

## Invariants

1. The `editorShell` object name, zero margins, tab-first/FindBar-second layout,
   and initial hidden FindBar state remain unchanged.
2. `DocumentTabSurface` and `FindSurface` retain their existing callback and
   semantic APIs; no document or editor policy moves into the composite.
3. `EditorShellSurface.set_locale()` routes to the owned FindSurface without
   creating a second locale model.
4. The surface imports only Qt, Locale, and existing presentation surfaces; it
   imports no application service, document state, persistence adapter, or
   TaskRunner.
5. MainWindow no longer imports or directly creates QWidget/QVBoxLayout for
   the central shell.

## Consequences

### Positive

- Central shell layout and child-surface parentage have one cohesive owner.
- MainWindow's semantic document/find policy is separated from widget layout
  without introducing a general UI framework.
- Future central-shell visual refinement can stay inside one presentation
  composition seam.

### Limits

- Qt startup, tab/FindBar interaction, focus, visual hierarchy, accessibility,
  DPI, and screen-reader behavior remain unrun under the project no-launch
  policy.
- This slice does not change document, editor, search, Replace All, close, save,
  recovery, or locale semantics.
- Public CloudWeGo references inform layering only; this ADR does not claim
  private ByteDance standards, certification, or release readiness.

## Public-source applicability

The architecture baseline records these public CloudWeGo references:

- CloudWeGo overview: <https://www.cloudwego.io/about/>.
- CloudWeGo open-source announcement:
  <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>.
- Kitex framework-extension guidance:
  <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>.

They are public engineering references, not target-specific manufacturer
requirements and not evidence of private ByteDance standards.

## Verification target

- Static source probe shows EditorShellSurface owns central QWidget/layout and
  MainWindow retains semantic callbacks and policy.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D27 review and handoff.
- Qt startup and interactive editor-shell behavior remain unverified.
