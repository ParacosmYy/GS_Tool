# ADR-0043: Extract the MainWindow settings surface

- **Status:** accepted-with-limits; D18 / ARCH-09 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly created `SettingsDialog`, executed its modal lifecycle,
and read the candidate `SettingsSnapshot` before handing persistence to
`SettingsService` and `TaskRunner`. The dialog is a presentation concern;
settings persistence, theme application, editor projection, and error policy
are application-facing MainWindow responsibilities.

## Decision

Add `quillforge.presentation.settings_surface.SettingsSurface`. The surface
owns the modal settings-dialog composition and exposes one semantic operation:
`edit(SettingsSnapshot) -> SettingsSnapshot | None`. `None` represents cancel
or dismissal; a snapshot is returned only after the dialog's existing Save
acceptance path.

MainWindow composes the surface and retains settings-service availability and
save-in-flight guards, operation IDs, worker submission, result validation,
theme application, locale refresh, editor preference projection, animation,
notifications, and error handling.

## Invariants

1. Save/cancel semantics and the immutable `SettingsSnapshot` payload remain
   source-equivalent.
2. `SettingsSurface` imports only Qt, domain `SettingsSnapshot`, and the
   existing presentation `SettingsDialog`; it owns no persistence or worker
   policy.
3. The surface owns the dialog parent relationship and modal execution, but it
   does not retain dialog state after `edit()` returns.
4. MainWindow remains the sole owner of settings persistence, asynchronous
   operation state, theme/editor projection, notification, and error policy.
5. No event bus, singleton, service locator, or speculative settings state
   container is introduced.

## Consequences

### Positive

- MainWindow no longer constructs or executes the concrete settings dialog.
- Settings presentation changes remain inside a focused presentation seam.
- The returned domain snapshot keeps the persistence boundary explicit and
  synchronous at the dialog edge.

### Limits

- MainWindow remains a large editor/recovery/session/plugin coordinator;
  subsequent slices remain bounded and incremental.
- Qt runtime dialog interaction, visual metrics, native fonts/DPI, and
  accessibility remain unrun under the no-launch policy.
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

- Static source probe shows no `SettingsDialog(...)` construction remains in
  MainWindow and `SettingsSurface.edit()` is the only dialog boundary.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D18 review and handoff.
- Qt startup and interactive settings behavior remain unverified.
