# ADR-0307: Workspace-search outside-workspace placeholder localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D263 / ARCH-241

## Context

The workspace-search diagnostics list correctly preserved the safety boundary
when an issue path could not be made relative to the selected root, but its
fallback label was hard-coded as `<outside selected workspace>`. The diagnostic
reason was localized separately, leaving one visible English placeholder in a
Chinese shell.

## Decision

Add `search.outside_workspace` to the existing presentation catalog and use it
only in `WorkspaceSearchDialog._relative_path` when `Path.relative_to` rejects
the path. English keeps the exact existing placeholder; Simplified Chinese
uses `<位于所选工作区之外>`. The existing diagnostic-row refresh path calls
the projection again after locale changes, so no translated state is cached.

## Boundaries and alternatives

This is a display-only projection. The issue path, root containment check,
diagnostic reason, result ordering, and expansion state remain unchanged. A
global message replacement would be less precise because this string is a
path-display fallback rather than an application error.

## Public-source applicability and review

Python's first-party [`Path.relative_to` documentation](https://docs.python.org/3/library/pathlib.html#pathlib.PurePath.relative_to)
is applicable to the existing containment-relative projection boundary. The
project workspace-search diagnostic and locale-refresh contracts are the
applicable engineering references. No manufacturer requirement changed.
Public CloudWeGo/ByteDance material remains an engineering reference only; no
private corporate standard, certification, MISRA, ISO 26262, ASPICE, or
embedded C/C++/MCU/RTOS claim is made.

