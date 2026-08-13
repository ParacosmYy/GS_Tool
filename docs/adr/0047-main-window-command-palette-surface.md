# ADR-0047: Extract the MainWindow command-palette surface

- **Status:** accepted-with-limits; D22 / ARCH-13 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly constructed and executed `CommandPaletteDialog`, then
interpreted its accepted state and selected command ID before resolving and
executing the live command registry entry. The dialog is presentation
composition; command lookup, stale-command handling, execution, and menu
refresh are coordinator/application policy.

## Decision

Add `quillforge.presentation.command_palette_surface.CommandPaletteSurface`.
The surface owns the modal dialog parent, locale, accepted/cancelled mapping,
and stable selected-command-ID projection through `choose(commands)`. It does
not execute commands or retain a second command registry.

MainWindow retains live command lookup, command execution, stale-command
notification, menu refresh, command registry ownership, and locale lifecycle.
`CommandPaletteDialog` also accepts its real Qt parent contract
(`QWidget | None`) rather than the narrower `QDialog | None` annotation.

## Invariants

1. Cancel, Escape, or dialog dismissal returns `None`; only explicit dialog
   acceptance returns the stable selected command ID.
2. CommandPaletteSurface passes the current command snapshot and locale into
   the existing dialog; it does not retain or mutate command state.
3. MainWindow resolves the returned ID again through `CommandRegistry` before
   execution, preserving stale/unregistered command handling.
4. Locale changes reach the surface through `_retranslate_ui()` and affect the
   next invocation without moving translation catalogs out of presentation.
5. No event bus, command executor, service locator, or speculative registry is
   introduced.

## Consequences

### Positive

- MainWindow no longer constructs or calls the concrete command-palette dialog.
- Modal composition and Qt parent typing are isolated behind a semantic seam.
- Command execution remains explicit and auditable in the coordinator.

### Limits

- Qt runtime palette interaction, keyboard/focus behavior, visual hierarchy,
  accessibility, DPI, and screen-reader output remain unrun under the
  no-launch policy.
- Command filtering/search behavior remains in the existing dialog and is not
  expanded by D22.
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

- Static source probe shows no `CommandPaletteDialog` import or direct modal
  execution in MainWindow and shows stable ID projection through the surface.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D22 review and handoff.
- Qt startup and interactive command-palette behavior remain unverified.
