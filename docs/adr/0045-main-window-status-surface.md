# ADR-0045: Extract the MainWindow status surface

- **Status:** accepted-with-limits; D20 / ARCH-11 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly created `StatusRail`, inserted it into the native status
bar, projected locale, and called widget phase methods from operation and
document-state policy. The status rail is a presentation projection; the
meaning of ready, working, attention, and error belongs to MainWindow's
application/UI coordination.

## Decision

Add `quillforge.presentation.status_surface.StatusSurface`. The surface owns
`StatusRail` creation and parent lifetime, exposes its widget for the status-bar
object tree, and provides semantic `set_locale()` and `set_phase()` methods.

MainWindow retains busy/TaskRunner/dirty-document phase precedence, operation
start/complete behavior, error projection, close guards, and notification
policy. `_sync_status_surface()` is the coordinator method name for that
policy-to-projection step.

## Invariants

1. Status phase precedence remains working > attention > ready, with explicit
   error projection preserved at the existing error boundary.
2. StatusSurface imports only Qt, domain `Locale`, and the existing
   presentation `StatusRail`; it owns no lifecycle or application policy.
3. The status-bar widget remains parented and inserted in the same native
   status bar; only the receiver changes from rail to surface.
4. Locale refresh and all operation/document/error status calls remain
   source-equivalent.
5. No event bus, singleton, service locator, or second status state model is
   introduced.

## Consequences

### Positive

- MainWindow no longer constructs or reaches into the concrete status widget.
- Status presentation and QSS/native projection changes remain in one seam.
- Lifecycle policy remains explicit and easy to audit in MainWindow.

### Limits

- MainWindow remains a large editor/recovery/session/plugin coordinator;
  subsequent slices remain bounded and incremental.
- Qt runtime status rendering, native metrics, accessibility, fonts, DPI, and
  cross-machine appearance remain unrun under the no-launch policy.
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

- Static source probe shows no `StatusRail(...)` construction or `_status_rail`
  field remains in MainWindow.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D20 review and handoff.
- Qt startup and interactive status rendering remain unverified.
