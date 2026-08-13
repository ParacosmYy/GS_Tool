# ADR-0044: Extract the MainWindow recovery prompt surface

- **Status:** accepted-with-limits; D19 / ARCH-10 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly assembled the recovery `QMessageBox`, formatted snapshot
timestamps, translated source-status explanations, and interpreted the clicked
button before dispatching to recovery policy. The prompt is presentation
composition; snapshot restore, discard scheduling, deferred session paths, and
notifications belong to MainWindow.

## Decision

Add `quillforge.presentation.recovery_prompt_surface.RecoveryPromptSurface`.
The surface owns prompt parentage, locale projection, path/time/source text,
button presentation, and returns the typed decision
`"restore" | "discard" | "later"`.

MainWindow maps the existing `RecoveryCandidate` to the surface's semantic
inputs and retains recovery-service availability, snapshot restore/discard
calls, deferred-path bookkeeping, session sequencing, document events, and
notifications.

## Invariants

1. Restore, discard, and later button roles map to the same three business
   branches; dismissal remains later/deferred.
2. RecoveryPromptSurface imports only Qt, domain `Locale`, filesystem `Path`,
   standard-library formatting, and presentation i18n; it imports no recovery
   service or infrastructure adapter.
3. Path display, untitled fallback, timestamp formatting, source-status
   translation, and current locale behavior remain source-equivalent.
4. MainWindow remains the sole owner of RecoveryService calls, deferred session
   paths, document events, cleanup scheduling, and notifications.
5. No event bus, singleton, service locator, or second recovery state model is
   introduced.

## Consequences

### Positive

- MainWindow no longer owns the concrete recovery prompt assembly and text
  formatting.
- Recovery prompt visual/locale changes stay inside a focused presentation
  seam while safety-sensitive recovery policy remains explicit.

### Limits

- MainWindow remains a large editor/recovery/session/plugin coordinator;
  subsequent slices remain bounded and incremental.
- Qt runtime prompt interaction, native metrics, accessibility, fonts, DPI,
  and recovery timing remain unrun under the no-launch policy.
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

- Static source probe shows no recovery prompt button construction remains in
  MainWindow; it only consumes the typed decision.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D19 review and handoff.
- Qt startup and interactive recovery behavior remain unverified.
