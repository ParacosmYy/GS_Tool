# ADR-0250: Workspace provider directory capability

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D202 / ARCH-188

## Context

`application.workspace.WorkspaceService` already depended on the
`WorkspaceProvider` port for directory enumeration, but it still called
`Path.is_dir()` directly in both workspace-open and directory-navigation
paths. `pathlib.Path.is_dir()` is a concrete filesystem I/O operation, so the
application layer owned part of the infrastructure predicate while the port
claimed to own filesystem mechanics.

## Decision

Add `WorkspaceProvider.is_directory(path: Path) -> bool` as the single
directory-capability query. `FileWorkspaceProvider` implements the query with
the existing `Path.is_dir()` behavior. `WorkspaceService` keeps path
normalization, containment, entry limits, and the existing user-facing
`ValueError` messages, but delegates the directory predicate through the
port. Workspace search, document path normalization, and directory
enumeration remain outside this bounded increment.

## Preserved invariants

- The only current provider preserves the prior `Path.is_dir()` true/false
  behavior, including the existing treatment of filesystem query failures.
- Workspace root validation, child containment, maximum-entry policy, and
  `list_directory` ordering remain application policy or provider behavior in
  their existing owners.
- Composition still wires `WorkspaceService(FileWorkspaceProvider())`; no
  second provider, service locator, or Qt dependency is introduced.
- Presentation signals, async dispatch, file opening, folder navigation, and
  user-visible error text remain unchanged.

## Review and applicability

The architecture consultation (`Singer the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Bacon the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one existing Port capability removes the direct filesystem predicate without
adding a parallel abstraction.

The applicable public first-party source is Python's `pathlib` documentation,
which distinguishes concrete paths and their OS-accessing operations,
including `Path.is_dir()`:
https://docs.python.org/3/library/pathlib.html#pathlib.Path.is_dir. This is a
Python 3.12 application/infrastructure boundary, not embedded C/C++, MCU,
RTOS, or manufacturer-requirement work; the mandatory embedded enterprise
workflow is therefore not applicable to this source slice. Public CloudWeGo
material remains engineering reference only; no private ByteDance standard,
certification, or compliance claim is made.

## Evidence and limits

- `D202-WORKSPACE-PREDICATE-PROBE=PASS port=1 adapter=1 app_direct_is_dir=0`
- `D202-COMPILEALL=PASS`
- `D202-RUFF=PASS`
- `D202-FORMAT=PASS`
- `D202-PRESENTATION-AUDIT=PASS`
- `D202-PACKAGE-BUILD-PS51=PASS`
- `D202-PACKAGE-BUILD-PS7=PASS`
- `D202-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native dialog, accessibility
tree, DPI, clean-machine, cross-machine, signing, installer, updater, legal,
support, or release-owner evidence was performed. Runtime provider behavior,
filesystem races, and external release gates remain open.
