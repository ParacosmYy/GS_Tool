# ADR-0057: Shared inline feedback state contract

- **Status:** accepted-with-limits; D32a / UI-18 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D31 gave the bottom status-bar notification a readable semantic hierarchy, but
the workspace panel and Find in Files dialog still render loading, successful,
warning, and recoverable-error text through the same muted label style. Users
must scan the wording instead of seeing the state immediately. These labels
already belong to presentation surfaces; the missing piece is a shared visual
state projection, not new application state.

## Decision

Add `presentation/feedback.py` as the single presentation-only contract for
`FeedbackLevel` (`info`, `working`, `success`, `warning`, `error`), the existing
D31 `StatusMessageLevel` subset, and dynamic Qt property repolishing. Reuse it
from `StatusSurface`, `WorkspacePanel`, and `WorkspaceSearchDialog`. Extend
centralized QSS selectors for the workspace and search status labels.

This slice does not change the FindBar call-site contract; FindBar semantic
status projection is explicitly deferred to D33 because its MainWindow call
sites need a separate, reviewable mapping of editor operation outcomes.

## Invariants

1. `FeedbackLevel` is presentation metadata only; it is not a domain or
   application lifecycle state and is never inferred from arbitrary text.
2. Existing workspace/search signals, callbacks, operation IDs, cancellation,
   root containment, result payloads, and localized messages remain unchanged.
3. `StatusSurface.StatusMessageLevel` remains import-compatible while using the
   shared state projection helper.
4. Every state remains distinguishable by text and geometry/border as well as
   color; contrast uses existing centralized theme tokens.
5. No widget imports application services, persistence, filesystem, or
   MainWindow policy. No worker mutates a widget.

## Alternatives considered

- **Duplicate `setProperty`/repolish code in each widget:** smaller initial
  diff, but it creates drift in the state contract and lifecycle behavior.
- **Import `StatusSurface` from every inline widget:** reuses D31 names but
  couples independent surfaces to a concrete composition surface; rejected.
- **Infer state by parsing status text:** fragile across locales and violates
  the explicit contract rule; rejected.

## Limits

Qt startup, native QSS specificity, visual rendering, screen-reader output,
font metrics, DPI, and cross-machine appearance remain unrun under the
no-launch policy. D33 FindBar semantic mapping remains open.

## Public-source applicability and embedded gate

This is a Python/PyQt6 presentation change, not embedded C/C++ or firmware.
The embedded-enterprise-workflow and embedded-code-review-simplifier are N/A
for MCU/vendor constraints. Public CloudWeGo sources remain transferable
engineering references only, not private ByteDance standards or certification:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static source review proves one shared feedback contract and no application
  imports in the helper or target surfaces.
- A targeted QSS probe covers workspace/search states and all supported theme
  combinations at >= 4.5:1 for normal text.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D32a handoff.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run.
