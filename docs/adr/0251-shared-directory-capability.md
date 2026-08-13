# ADR-0251: Shared directory capability for workspace search

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D203 / ARCH-189

## Context

After D202, workspace navigation used the `WorkspaceProvider` directory
capability, but `WorkspaceSearchService` and `FileWorkspaceSearchProvider`
still owned a second direct `Path.is_dir()` path. That left the application
layer with a concrete filesystem predicate and invited duplicate contracts for
two related local-directory use cases.

## Decision

Define one framework-neutral `DirectoryCapability` Protocol in
`application.ports`. `WorkspaceProvider` and `WorkspaceSearchProvider`
inherit that capability. `WorkspaceSearchService` asks its provider whether
the normalized root is a directory, and `FileWorkspaceSearchProvider`
implements the same capability and reuses it in its search entry point.

## Preserved invariants

- The file search adapter still uses `Path.is_dir()` for the same root check;
  the existing invalid-root `ValueError` text is unchanged.
- Search cancellation, bounded file/byte/depth/result/diagnostic limits,
  symlink/reparse-point handling, result-path containment, and diagnostics are
  unchanged.
- D202 workspace navigation keeps the same provider capability and behavior;
  composition and diagnostic composition continue to wire concrete adapters
  directly into application services.
- No new filesystem service, service locator, UI dependency, or second
  directory contract is introduced.

## Review and applicability

The architecture consultation (`Nash the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. The independent review (`Turing the 6th / Luna max`) likewise
returned no conclusion after two bounded waits and was closed. Parent review
is `PASS`, and the behavior-preserving simplification assessment is `PASS`:
one shared Protocol removes duplicate capability declarations without moving
search policy or adding a new service.

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

- `D203-DIRECTORY-CAPABILITY-PROBE=PASS shared=1 workspace=1 search=1 application_is_dir=0`
- `D203-COMPILEALL=PASS`
- `D203-RUFF=PASS`
- `D203-FORMAT=PASS`
- `D203-PRESENTATION-AUDIT=PASS`
- `D203-PACKAGE-BUILD-PS51=PASS`
- `D203-PACKAGE-BUILD-PS7=PASS`
- `D203-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native dialog, accessibility
tree, DPI, clean-machine, cross-machine, signing, installer, updater, legal,
support, or release-owner evidence was performed. Live filesystem races,
provider runtime behavior, and external release gates remain open.
