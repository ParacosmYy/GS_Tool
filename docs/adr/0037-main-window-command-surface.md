# ADR-0037: Extract the MainWindow command surface coordinator

- **Status:** accepted-with-limits; Phase 2 first slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` is the Qt shell and currently coordinates documents, workspace
operations, recovery, sessions, plugins, settings, editor actions, and the
menu/command-rail projection. The command surface is a distinct presentation
responsibility: it maps the application-owned `CommandRegistry` into menus and
toolbar actions, then reprojects those widgets when plugins or locale change.

Keeping that projection inline makes every visual or command-surface change
touch the lifecycle coordinator. It also makes the Qt ownership and command
refresh contract harder to inspect independently.

## Decision

Add `quillforge.presentation.command_surface` with:

- `ToolbarActionSpec`, an immutable presentation-only description of one
  command-rail action and its callback;
- `CommandSurface`, the owner of menu and toolbar widgets, action refresh, and
  locale retranslation.

`MainWindow` remains responsible for registering core commands and owning the
application callbacks. Its existing `refresh_command_menus()` method remains
as the stable lifecycle seam and delegates to `CommandSurface`. The surface
does not know document, workspace, plugin, or domain policy; it only invokes
the callback supplied by the owner.

## Invariants

1. `CommandRegistry` remains application-owned and is the sole source of
   registered commands.
2. Menu actions retain command IDs, shortcut strings, ordering, and callback
   behavior; unknown menu IDs remain ignored as before.
3. The toolbar keeps the existing action order, separator, standard icons,
   workspace conditional action, object names, and locale projection.
4. Qt actions remain parented to the existing `MainWindow`; refresh disables
   and schedules old actions for deletion before rebuilding them.
5. The coordinator imports only presentation, application-command, domain
   locale, and Qt types; it does not import infrastructure or business policy.

## Consequences

### Positive

- Menu/toolbar visual changes have one focused presentation module.
- MainWindow keeps the stable callback and lifecycle contract while becoming
  incrementally smaller.
- Plugin refresh and settings locale changes share one explicit projection
  boundary.
- Future command-surface variants can be composed without duplicating command
  registration or application behavior.

### Limits

- MainWindow remains a large coordinator; document, workspace, recovery,
  session, and plugin coordinators are separate future slices.
- Runtime Qt interaction, visual screenshots, and accessibility output remain
  unrun under the project no-launch policy.
- This is not a claim of complete enterprise migration or ByteDance private
  standard compliance.

## Alternatives rejected

- A command-surface service in the application layer: it would leak Qt widgets
  into the use-case boundary.
- Moving core command registration into the surface: it would make the
  presentation layer own application behavior and plugin policy.
- A generic event bus or dependency-injection container: unnecessary for this
  focused projection seam and would hide ownership.
- A wholesale MainWindow rewrite: too broad for an evidence-backed slice.

## Verification

- Static import/boundary probe confirms MainWindow delegates projection and the
  new coordinator owns the old surface state.
- Compile, lint, format, handoff, package provenance, and release no-go checks
  are recorded in the D12 review and handoff.
