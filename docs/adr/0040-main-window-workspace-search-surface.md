# ADR-0040: Extract the MainWindow workspace-search surface

- **Status:** accepted-with-limits; D15 / ARCH-06 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` previously created `WorkspaceSearchDialog`, bound its three
semantic signals, managed dialog activation, and directly projected locale,
busy/cancellation feedback, results, and recoverable errors. The same class
also owns the application search service, `TaskRunner`, operation IDs,
generation guards, cooperative cancellation, workspace containment, session
restore ordering, and user notifications.

The dialog assembly and Qt projection are presentation responsibilities. Search
execution state and result policy must remain in `MainWindow`: they coordinate
`WorkspaceSearchService`, stale completion rejection, workspace root changes,
and document activation.

## Decision

Add `quillforge.presentation.workspace_search_surface.WorkspaceSearchSurface`
and its explicit `WorkspaceSearchSurfaceCallbacks` contract. The surface owns:

- creation and parent lifetime of the non-modal `WorkspaceSearchDialog`;
- binding the search, cancel, and file-result signals to shell callbacks;
- root, locale, show/raise/activate, busy/cancel-feedback, result, error, and
  cancelled projection methods.

`MainWindow` stores the surface rather than the dialog and delegates only those
projection calls. It continues to own:

- `WorkspaceSearchService` query construction and `TaskRunner` submission;
- operation ID, generation, cancellation-event, and stale-completion rules;
- workspace-root containment checks before opening a result;
- notifications, error policy, and session/workspace coordination.

## Invariants

1. `WorkspaceSearchSurfaceCallbacks` exposes exactly the existing three
   semantic routes and preserves their payload shapes: `(str, bool)`, `()`,
   and `(object, int)`.
2. The surface imports only domain and presentation types; it does not select
   an application service, touch infrastructure, create a worker, or own
   search policy.
3. Switching roots invalidates the current search in `MainWindow` before the
   surface clears stale result rows, preserving the previous lifecycle order.
4. Locale refresh, busy/cancel feedback, result/error/cancelled projection, and
   non-modal activation remain available through one explicit surface boundary.
5. `MainWindow` remains the only owner of query validation, async lifecycle,
   generation rejection, root containment, document opening, and notification
   semantics.
6. No generic event bus, singleton, widget registry, second search state model,
   or dependency-injection framework is introduced.

## Consequences

### Positive

- Search dialog construction and external signal wiring have one inspectable
  presentation owner.
- `MainWindow` no longer knows the dialog's Qt activation sequence or widget
  construction details.
- Search policy remains close to the application service and lifecycle guards,
  while future search styling can evolve inside presentation.

### Limits

- `MainWindow` remains a large coordinator; recovery, session, and plugin
  coordinators remain future slices.
- Qt runtime signal delivery, dialog parenting, focus/activation, visual
  appearance, and file-result interaction remain unrun under the no-launch
  policy.
- Public CloudWeGo material is an engineering reference for layering and
  ownership only; this ADR does not claim a private ByteDance standard,
  certification, or release readiness.

## Alternatives rejected

- Moving `WorkspaceSearchService` or `TaskRunner` into the surface would mix
  Qt projection with application lifecycle and stale-result policy.
- Keeping all dialog wiring inline would leave presentation assembly coupled to
  every future MainWindow operation change.
- Moving cancellation/generation into the surface would duplicate the
  workspace operation guards and make root changes harder to reason about.

## Public-source applicability

The architecture baseline records these public CloudWeGo references:

- CloudWeGo overview: <https://www.cloudwego.io/about/>.
- CloudWeGo open-source announcement: <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>.
- Kitex framework-extension guidance: <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>.

They are public engineering references, not target-specific manufacturer
requirements and not evidence of private ByteDance standards.

## Verification target

- Static source probe shows no `WorkspaceSearchDialog(...)` construction or
  workspace-search signal binding remains in `MainWindow`.
- The new surface has no application/infrastructure imports and contains only
  presentation composition/projection.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D15 review and handoff.
- Qt startup and interactive search/file activation remain explicitly
  unverified until authorized.
