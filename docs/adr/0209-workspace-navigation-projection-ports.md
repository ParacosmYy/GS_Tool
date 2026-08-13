# ADR-0209: workspace-navigation projection Ports contract

- Status: accepted-with-limits
- Date: 2026-08-13
- Delivery: D160 / ARCH-147

## Context

`WorkspaceNavigationProjectionCoordinator` already isolated the final
projection of validated workspace-open and directory results, but its
constructor accepted nine independent callbacks. File/folder activation and
workspace service ownership were stable while positional wiring made the
navigation projection order harder to review.

## Decision

Introduce the frozen/slotted Qt-free `WorkspaceNavigationProjectionPorts`
contract with named callbacks for search invalidation, workspace activation,
search-root projection, surface lookup, directory projection, opened
notification, session save, restore continuation, and current-root lookup.
Preserve the existing order:

`invalidate search -> activate workspace -> set search root -> surface guard ->
set directory -> notify opened -> request session save -> finish session
restore`.

Directory results remain guarded by current root and surface presence.
MainWindow retains WorkspaceService, containment, document/file activation,
TaskRunner, surface, session, startup, close, and concrete Qt ownership. No Qt
type enters the coordinator.

## Alternatives rejected

- Keeping positional callbacks would retain avoidable workspace navigation
  wiring risk.
- Moving WorkspaceService, containment, file activation, or session policy into
  Ports would violate existing ownership boundaries.
- Introducing an event bus or navigation state machine would add lifecycle
  complexity without changing behavior.

## Review and evidence

Sartre the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Kepler the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion.
No child PASS is claimed. Parent review is `PASS`; simplification assessment
is `PASS` because the immutable named contract removes positional coupling
without adding behavior, navigation, or policy.

Authorized non-destructive evidence:

- `D160-WORKSPACE-NAVIGATION-BRANCH-PROBE=PASS`
- `D160-OPENED-DIRECTORY-ORDER-SHORT-CIRCUIT-PROBE=PASS`
- `D160-PORTS-IMMUTABILITY-PROBE=PASS`
- `D160-SOURCE-WIRING-PROBE=PASS`
- `D160-QT-FREE-CONTRACT-PROBE=PASS`
- `D160-PRESENTATION-AUDIT=PASS`
- `D160-COMPILEALL=PASS`
- `D160-RUFF=PASS`
- `D160-FORMAT=PASS`
- `D160-PACKAGE-BUILD=PASS`
- `D160-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `177B037CB066DC976E5DA4F22F59AD59971646475A176F7DAE035C5D10ED62FB`
- bytes: `38547989`
- source revision: `tree-sha256:b72e02761e3b409da9393f8a5119f4e563c68d5667e9ca44a34758cec5d39c22`

Public-source applicability is Python 3.12/PyQt6 presentation architecture;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply.
Public CloudWeGo material remains an engineering reference only, not a
private ByteDance standard or certification/compliance claim.

## Limits

The inline and static/package checks do not prove native event timing,
file/folder activation, filesystem behavior, runtime startup, clean-machine or
cross-machine behavior, signing, installer, update, legal clearance, support
ownership, or release readiness. Those gates remain open under the active
no-launch/no-release authorization boundary.
