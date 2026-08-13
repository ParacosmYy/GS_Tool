# ADR-0041: Close the MainWindow workspace projection boundary

- **Status:** accepted-with-limits; D16 / ARCH-07 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D14 moved workspace dock/panel construction and signal wiring into
`WorkspaceSurface`, but MainWindow still reached through a compatibility
`_workspace_panel` property to call `set_loading`, `set_directory`,
`show_error`, and `current_path`. That left the shell coupled to the concrete
`WorkspacePanel` widget even though the surface already owned its lifetime.

Those calls are presentation projection concerns. Workspace service calls,
TaskRunner lifecycle, busy/operation guards, generation/cancellation, session
restore ordering, and root containment remain MainWindow/application concerns.

## Decision

Extend `WorkspaceSurface` with a small semantic projection API:

- read-only `current_path`;
- `set_loading(bool)`;
- `set_directory(WorkspaceDirectory, Path)`; and
- `show_error(str)`.

Remove MainWindow's `WorkspacePanel` import, `_workspace_panel` property, and
direct widget calls. MainWindow uses only `WorkspaceSurface` for workspace
visual results while retaining all existing service and lifecycle policy.

## Invariants

1. The new methods are thin one-to-one projections onto the surface's sole
   panel; no second directory state, error policy, or async state is added.
2. `WorkspaceSurface` imports only Qt, domain models, i18n, and the existing
   presentation panel; no application or infrastructure dependency moves into
   the surface.
3. Existing workspace callback payloads, operation completion ordering,
   generation rejection, cancellation behavior, session-restore barrier, and
   root containment remain unchanged.
4. `MainWindow` contains no `WorkspacePanel` or `_workspace_panel` reference;
   panel lifetime and widget-specific projection stay inside the surface.
5. No service locator, singleton, event bus, widget registry, or dependency
   injection framework is introduced.

## Consequences

### Positive

- MainWindow depends on the semantic workspace surface rather than the
  concrete navigation widget.
- Workspace presentation ownership is complete for both user intents and
  current application-result projection.
- Future panel replacement or styling can remain within presentation without
  changing workspace operation code.

### Limits

- MainWindow remains the owner of workspace async policy and is still a large
  coordinator; recovery, session, and plugin slices remain future work.
- Qt runtime projection, panel lifetime, native metrics, and interactive
  workspace behavior remain unrun under the no-launch policy.
- Public CloudWeGo references inform layering only; this decision does not
  claim private ByteDance standards, certification, or release readiness.

## Public-source applicability

The architecture baseline records these public CloudWeGo references:

- CloudWeGo overview: <https://www.cloudwego.io/about/>.
- CloudWeGo open-source announcement: <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>.
- Kitex framework-extension guidance: <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>.

They are public engineering references, not target-specific manufacturer
requirements and not evidence of private ByteDance standards.

## Verification target

- Static source probe shows no `WorkspacePanel` import or `_workspace_panel`
  property/reference in MainWindow.
- The four semantic projection methods have the existing domain types and
  one-to-one delegation.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D16 review and handoff.
- Qt startup and interactive workspace behavior remain explicitly unverified.
