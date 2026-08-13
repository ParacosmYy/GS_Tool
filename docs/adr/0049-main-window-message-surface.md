# ADR-0049: Extract the MainWindow message surface

- **Status:** accepted-with-limits; D24 / ARCH-15 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly composed three different `QMessageBox` flows: the
save-before-close decision, the About dialog, and recoverable error dialogs.
The close/save policy, error lifecycle phase, and status projection belong to
the coordinator, while common modal composition and localization are
presentation concerns that should have one seam.

## Decision

Add `quillforge.presentation.message_surface.MessageSurface`. The surface
owns the parented, localized composition of save-before-close, About, and
recoverable error messages. It returns the typed
`save | discard | cancel` decision for close confirmation and performs no
document, save, close, status, or operation policy.

`MainWindow` retains dirty-state checks, Save As selection, asynchronous save
dispatch, tab removal, error phase projection, and status synchronization.

## Invariants

1. Save, Discard, and Cancel map one-to-one to the stable
   `SaveBeforeCloseDecision` values; any unexpected QMessageBox result fails
   closed to `cancel`.
2. `set_locale()` updates the locale used by the next message, and
   `MainWindow._retranslate_ui()` remains the single shell refresh route.
3. MessageSurface imports no application service, persistence adapter,
   document state, or task runner.
4. MainWindow does not import or construct `QMessageBox`; it retains all
   consequences of each returned decision and keeps error status projection
   around the modal call.
5. No new event bus, global singleton, dialog policy, or second operation
   state model is introduced.

## Consequences

### Positive

- Common message composition and localization have one presentation seam.
- Save-before-close, About, and error dialogs can receive future theme and
  accessibility refinement without expanding MainWindow's policy surface.
- The typed decision makes cancellation and save/discard consequences explicit
  at the coordinator boundary.

### Limits

- Qt startup, modal interaction, focus, visual hierarchy, accessibility, DPI,
  and screen-reader behavior remain unrun under the project no-launch policy.
- This slice does not change save, close, error, or status behavior and does
  not claim runtime visual acceptance.
- Public CloudWeGo references inform layering only; this ADR does not claim
  private ByteDance standards, certification, or release readiness.

## Public-source applicability

The architecture baseline records these public CloudWeGo references:

- CloudWeGo overview: <https://www.cloudwego.io/about/>.
- CloudWeGo open-source announcement:
  <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>.
- Kitex framework-extension guidance:
  <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>.

They are public engineering references, not target-specific manufacturer
requirements and not evidence of private ByteDance standards.

## Verification target

- Static source probe shows MessageSurface owns `question`, `about`, and
  `critical` composition while MainWindow has no QMessageBox import or call.
- The close flow consumes the typed decision, preserves Save As and async-save
  ownership, and keeps error phase/status synchronization in MainWindow.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D24 review and handoff.
- Qt startup and interactive message behavior remain unverified.
