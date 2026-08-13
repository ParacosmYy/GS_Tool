# ADR-0042: Extract the MainWindow Find surface

- **Status:** accepted-with-limits; D17 / ARCH-08 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly created `FindBar`, connected six semantic signals, hid
the widget, placed it in the editor shell, retranslations, and directly called
its query/status/operation methods. The same class owns the active-document
lookup, literal find/replace policy, Replace All session, cancellation,
content-version guards, tab locking, and status/error semantics.

The Qt widget assembly and projection methods are presentation concerns.
Editor behavior and cooperative operation policy must remain in MainWindow.

## Decision

Add `quillforge.presentation.find_surface.FindSurface` and its explicit
`FindSurfaceCallbacks` contract. The surface owns:

- `FindBar` construction and parent lifetime;
- six existing semantic signal routes;
- shell widget exposure, hide/show-find mode, locale, query/replacement/
  case access, status, operation-active, and session-reset projection.

MainWindow composes the surface and retains active tab resolution, editor
search/replace calls, Replace All session state, cancellation/rollback,
content-version validation, tab locking, and notification/error policy.

## Invariants

1. The six callback shapes remain `(bool)`, `()`, `()`, `()`, `()`, and `()`
   for find, replace, replace-all, cancel, close, and criteria-change routes.
2. `FindSurface` imports only Qt, domain `Locale`, and the existing
   presentation `FindBar`; it owns no editor, application, or infrastructure
   policy.
3. The surface delegates query, replacement, case, status, locale, busy,
   reset, and mode behavior one-to-one without duplicating widget state.
4. MainWindow remains the sole owner of `_find_match`, Replace All sessions,
   operation IDs, rollback, tab locking, editor calls, and status/error policy.
5. No event bus, singleton, widget registry, service locator, or speculative
   coordinator framework is introduced.

## Consequences

### Positive

- MainWindow no longer constructs or wires the concrete FindBar widget.
- Find/Replace layout and semantic projection have one inspectable owner.
- Future find-bar visual changes remain within presentation without moving
  editor behavior into a widget coordinator.

### Limits

- MainWindow remains a large editor/recovery/session/plugin coordinator;
  further slices remain bounded and incremental.
- Qt runtime signal delivery, focus/keyboard behavior, native metrics, and
  Replace All interaction remain unrun under the no-launch policy.
- Public CloudWeGo references inform layering only; this ADR does not claim
  private ByteDance standards, certification, or release readiness.

## Public-source applicability

The architecture baseline records these public CloudWeGo references:

- CloudWeGo overview: <https://www.cloudwego.io/about/>.
- CloudWeGo open-source announcement: <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>.
- Kitex framework-extension guidance: <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>.

They are public engineering references, not target-specific manufacturer
requirements and not evidence of private ByteDance standards.

## Verification target

- Static source probe shows no `FindBar(...)` construction or six external
  signal bindings remain in MainWindow.
- The surface API and callback payloads are source-equivalent.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D17 review and handoff.
- Qt startup and interactive Find/Replace behavior remain unverified.
