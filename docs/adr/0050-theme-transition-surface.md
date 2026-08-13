# ADR-0050: Extract the theme transition surface

- **Status:** accepted-with-limits; D25 / ARCH-16 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

After appearance settings are persisted, `MainWindow` selected the central
widget and directly created the `QGraphicsOpacityEffect` and
`QPropertyAnimation` used to fade the shell in. The motion preference and the
time at which a transition is requested are coordinator policy, while the
animation object graph and cleanup are presentation composition details.

## Decision

Add `quillforge.presentation.theme_transition_surface.ThemeTransitionSurface`.
The surface owns the short-lived opacity effect, easing, animation lifetime,
interrupted-animation replacement, and finished cleanup. It accepts a target
widget and does not know settings, themes, editors, persistence, or
application operations.

`MainWindow` retains the `motion_enabled` decision, central-widget selection,
and invocation after the existing theme/editor projection.

## Invariants

1. Motion-disabled settings do not create or start an animation.
2. A missing central target is a no-op.
3. The existing 220 ms, 0.72 → 1.0, OutCubic transition remains unchanged.
4. A new transition stops the previous animation before installing its new
   effect; the finished callback removes only the effect it created.
5. The surface introduces no settings state, theme policy, editor policy,
   timer, singleton, event bus, or application dependency.

## Consequences

### Positive

- MainWindow no longer owns Qt animation primitives or their cleanup closure.
- Animation behavior has one reusable seam for future reduced-motion and
  transition-style refinements.
- The current user preference and theme application order remain explicit in
  MainWindow.

### Limits

- Qt startup, animation timing, effect rendering, focus, DPI, accessibility,
  and visual acceptance remain unrun under the project no-launch policy.
- This slice does not change theme tokens, font settings, editor behavior, or
  settings persistence.
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

- Static source probe shows ThemeTransitionSurface owns animation primitives
  and MainWindow owns only motion gating, target selection, and trigger timing.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D25 review and handoff.
- Qt startup and interactive animation behavior remain unverified.
