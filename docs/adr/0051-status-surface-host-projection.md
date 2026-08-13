# ADR-0051: Complete the status surface host boundary

- **Status:** accepted-with-limits; D26 / ARCH-17 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`StatusSurface` already owned the semantic `StatusRail`, locale, and phase
projection, but `MainWindow` still configured the `QStatusBar` size grip,
attached the permanent rail, and localized/transmitted transient notifications
directly. That left the status presentation boundary split across the shell
coordinator.

## Decision

Extend `quillforge.presentation.status_surface.StatusSurface` with
`attach_to(QStatusBar)` and `show_message(message, timeout_ms=5000)`. The
surface owns status-bar host composition, its size-grip choice, notification
localization, and the transient message projection. `set_locale()` updates the
surface's locale used by both the rail and notifications.

`MainWindow` retains the status phase precedence, TaskRunner/document policy,
notification timing decisions supplied by its existing call sites, and the
single host selection during initialization.

## Invariants

1. `attach_to()` is idempotent for the same status bar and can detach the rail
   from an old host before attaching a new one.
2. The permanent status rail remains the same `StatusRail` instance and keeps
   its semantic phase projection.
3. Notifications use the current normalized surface locale and preserve the
   existing 5000 ms default timeout; negative custom timeouts clamp to zero.
4. StatusSurface imports no application service, document state, TaskRunner,
   persistence adapter, or operation policy.
5. MainWindow no longer calls `QStatusBar.setSizeGripEnabled()`,
   `addPermanentWidget()`, or `showMessage()` directly.

## Consequences

### Positive

- Status-bar host composition, phase projection, and transient notification
  localization now share one high-cohesion presentation seam.
- Future status rail and notification visual refinement does not require
  expanding MainWindow's Qt surface.
- Locale refresh remains one explicit `StatusSurface.set_locale()` path.

### Limits

- Qt startup, status-bar rendering, notification timing, focus, accessibility,
  DPI, and screen-reader behavior remain unrun under the project no-launch
  policy.
- This slice does not change phase precedence, worker lifecycle, document
  policy, or notification call-site semantics.
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

- Static source probe shows StatusSurface owns status-bar attachment and
  localized notification projection while MainWindow retains phase/policy.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D26 review and handoff.
- Qt startup and interactive status-bar behavior remain unverified.
