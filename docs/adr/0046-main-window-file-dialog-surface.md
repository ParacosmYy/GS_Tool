# ADR-0046: Extract the MainWindow file-dialog surface

- **Status:** accepted-with-limits; D21 / ARCH-12 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly called three native Qt dialog APIs for distinct user
intentions: opening one document, choosing one workspace folder, and choosing
a save destination. Keeping these calls in the large coordinator made the
reported file-versus-folder behavior harder to audit and coupled native dialog
details to asynchronous application policy.

## Decision

Add `quillforge.presentation.file_dialog_surface.FileDialogSurface`. The
surface owns the native `QFileDialog` calls, localized titles and filters, and
conversion of a selected string into `Path | None`. It exposes three semantic
methods: `choose_document()`, `choose_workspace()`, and
`choose_save_path(current)`.

MainWindow retains busy/startup guards, asynchronous document opening,
workspace service dispatch, save policy, session/recovery paths, and all
notifications/errors. Explicit document open and Save As remain allowed to
select an arbitrary local path; workspace-tree and search-result containment
policies are unchanged and remain outside this surface.

## Invariants

1. `choose_document()` uses `QFileDialog.getOpenFileName`; it does not use the
   directory-only API.
2. `choose_workspace()` uses `QFileDialog.getExistingDirectory`; it cannot
   accidentally route a folder through document opening.
3. `choose_save_path()` preserves the existing `Untitled.txt` fallback and
   current-path default while returning `None` on cancellation.
4. Locale changes reach the surface through MainWindow's existing
   `_retranslate_ui()` seam; dialog localization remains presentation-only.
5. No workspace containment, document I/O, session, recovery, or async state is
   introduced into the surface.

## Consequences

### Positive

- File, folder, and save destination semantics are explicit and source-auditable.
- MainWindow no longer imports or constructs `QFileDialog` directly.
- Native dialog styling/behavior can evolve behind one presentation boundary
  without moving application policy.

### Limits

- Qt runtime file-dialog interaction, native filters, DPI, accessibility, and
  visual behavior remain unrun under the no-launch policy.
- The file filter remains a user-facing hint; `All files (*.*)` and downstream
  document-service validation are unchanged.
- Cross-workspace policy, symlink/reparse final identity, and session restore
  containment are not changed by D21.
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

- Static source probe shows no `QFileDialog` import or call in MainWindow and
  shows all three semantic methods in FileDialogSurface.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D21 review and handoff.
- Qt startup and native dialog interaction remain unverified.
