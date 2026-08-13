# ADR-0183: workspace-search surface admission boundary

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D140 / ARCH-121

## Context

`MainWindow._show_workspace_search` combined startup-restore admission,
service/workspace availability, missing-root feedback, creation/reuse of the
non-modal search surface, root-change invalidation, and presentation. Query
validation, operation tracking, cancellation, worker dispatch, and result
classification already belong to the existing workspace-search components and
must not move with this surface boundary.

## Decision

Add the Qt-free `WorkspaceSearchSurfaceAdmissionCoordinator`, its frozen/slotted
`WorkspaceSearchSurfaceAdmissionPorts`, and a small
`WorkspaceSearchSurfacePort` protocol. Preserve this exact sequence:

1. reject startup restore with the exact existing warning;
2. reject missing search service/workspace with the existing error;
3. reject a workspace without a root with the existing warning;
4. create the existing Qt surface once, or reuse it;
5. invalidate active search and reset the surface root when the workspace root
   changes; and
6. show the existing surface.

`WorkspaceSearchSurface`, query validation, operation tracker, cancellation,
`WorkspaceSearchCoordinator`, result projection, containment, locale refresh,
notifications, and close policy remain in their existing owners. The new
coordinator has no PyQt6 import and does not own a worker or search service.

## Alternatives rejected

- Moving `WorkspaceSearchSurface` or `WorkspaceSearchDialog` construction into
  the Qt-free coordinator would violate the presentation boundary.
- Moving query validation or operation submission here would duplicate the
  existing search-operation boundary.
- A generic “surface lifecycle” abstraction would obscure the workspace-root
  policy and add coupling without another concrete consumer.

## Review and evidence

Ptolemy the 4th / Luna max was assigned the architecture assessment and
returned no conclusion within the bounded review window. Singer the 4th / Luna
max was assigned the independent read-only review and also returned no
conclusion. No child PASS is claimed. Parent review is `PASS`; simplification
assessment is `PASS` because the coordinator isolates only the existing
surface admission sequence and leaves search execution in place.

Authorized non-destructive evidence: `D140-WORKSPACE-SEARCH-SURFACE-
BEHAVIOR-PROBE=PASS`, `D140-WORKSPACE-SEARCH-SURFACE-CONTRACT-PROBE=PASS`,
`D140-QT-FREE-WORKSPACE-SEARCH-PROBE=PASS`,
`D140-MAINWINDOW-WORKSPACE-SEARCH-WIRING-PROBE=PASS`, compile/lint/format,
project checks, package identity, no-launch, traceability, and expected release
NO-GO. Native search dialog, filesystem behavior, worker timing, and runtime
visual evidence remain unrun under the active authorization boundary.

Public-source applicability is Python 3.12/PyQt6 presentation orchestration;
no embedded C/C++, MCU, RTOS, or manufacturer requirement applies. No external
vendor rule was used as a conformance claim. Public CloudWeGo material remains
an engineering reference only, not a private ByteDance standard or a
certification/compliance claim.
