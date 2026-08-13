# ADR-0039: Extract the MainWindow workspace surface

- **Status:** accepted-with-limits; D14 / ARCH-05 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` currently creates the workspace `QDockWidget` and
`WorkspacePanel`, binds five semantic panel signals, owns their Qt parent
relationship, and retranslates both the dock and panel. The same class also
owns workspace service calls, generation/operation guards, cancellation,
root-containment checks, session-restore barriers, and workspace-search
results.

The Qt assembly is a presentation responsibility. The asynchronous operation
state and application policy must remain in MainWindow because they coordinate
`WorkspaceService`, `TaskRunner`, session restore, and document activation.

## Decision

Add `quillforge.presentation.workspace_surface.WorkspaceSurface` and its
explicit `WorkspaceSurfaceCallbacks` contract. The surface owns:

- creation and lifetime parenting of `WorkspacePanel` and `QDockWidget`;
- binding the panel's folder, directory, file, back, and cancel signals to
  callbacks supplied by the shell; and
- dock/panel locale projection while preserving the visible directory page.

MainWindow creates the surface only when workspace support is composed and
retains a read-only panel access seam for projecting current application
results. It continues to own service calls, operation IDs/generations,
cooperative cancellation, containment validation, session-restore ordering,
search dialog behavior, user notifications, and error policy.

## Invariants

1. `WorkspaceSurfaceCallbacks` contains semantic intents only; it does not
   expose application services, mutable containers, or Qt widgets to callers.
2. The surface imports presentation/domain types only; no application or
   infrastructure dependency moves outward into the presentation layer.
3. The five existing `WorkspacePanel` signal routes keep their callback
   targets and payload shapes unchanged.
4. Locale changes update both dock title and panel labels through one surface
   method; directory data and current-page state remain owned by
   `WorkspacePanel`.
5. MainWindow remains the only owner of workspace service lifecycle, stale
   result rejection, cancellation, root containment, and session barriers.
6. No second workspace state model, service locator, singleton, event bus, or
   dependency-injection framework is introduced.

## Consequences

### Positive

- Workspace dock composition and signal wiring have one inspectable owner.
- MainWindow's asynchronous workspace policy is easier to distinguish from
  the visual projection.
- Future workspace surface styling or dock placement changes can stay within
  presentation without changing filesystem/application contracts.
- The explicit callback record makes the integration boundary difficult to
  misuse and keeps signal payload ownership visible.

### Limits

- MainWindow still owns workspace operations and remains a large coordinator;
  recovery, session, search, and plugin coordinators remain future slices.
- Qt runtime signal delivery, dock parenting, visual appearance, and
  accessibility output remain unrun under the no-launch policy.
- This is a project architecture decision informed by public engineering
  references, not a claim of private ByteDance standard compliance,
  certification, or release readiness.

## Alternatives rejected

- Moving workspace service calls into `WorkspaceSurface`: this would leak
  application policy and async lifecycle into the visual layer.
- Keeping signal binding inline: it would leave creation/wiring coupled to every
  future MainWindow operation change.
- A generic signal bus or widget registry: unnecessary for five explicit
  semantic routes and would hide ownership.
- Extracting all workspace/search state at once: too broad for a reviewable
  vertical slice.

## Verification target

- Static boundary probe must show no `QDockWidget` construction or
  `WorkspacePanel(...)` construction remains in MainWindow.
- Compile/lint/format, handoff, package provenance, and release no-go evidence
  must be recorded in the D14 review and handoff.
- Qt startup and interactive workspace/file behavior remain explicitly
  unverified until the user authorizes launch.
