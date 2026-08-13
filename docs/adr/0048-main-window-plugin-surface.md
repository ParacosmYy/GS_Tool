# ADR-0048: Extract the MainWindow plugin surface

- **Status:** accepted-with-limits; D23 / ARCH-14 bounded Phase 2 slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

`MainWindow` directly constructed, connected, showed, raised, activated, and
retranslated both the extension-catalog dialog and the registered-plugin
status dialog. It also reached into the catalog dialog to disable governance
buttons while an approval worker was in flight. These are presentation
composition details; scan, approval, enablement, operation IDs, and failure
policy belong to MainWindow/application services.

## Decision

Add `quillforge.presentation.plugin_surface.PluginSurface`. The surface owns
catalog/status dialog instances, replacement and activation, locale refresh,
four semantic callback routes, and catalog governance-action projection.

MainWindow retains PluginCatalogService/PluginApprovalService/PluginRuntime,
TaskRunner submission, operation/generation state, result validation,
enablement/approval policy, notifications, and error behavior.

## Invariants

1. A new catalog or status projection closes the previous dialog before the
   replacement is shown, raised, and activated.
2. Approval/revocation and enable/disable signals route one-to-one to existing
   MainWindow callbacks; the surface does not execute governance operations.
3. Locale updates reach both open dialogs and the next dialog construction
   through one `PluginSurface.set_locale()` seam.
4. Governance button enablement remains a projection of MainWindow's existing
   worker state; the surface does not create a second inflight state.
5. No plugin loading, trust promotion, external execution, event bus, or
   service locator is introduced.

## Consequences

### Positive

- MainWindow no longer imports or constructs the two concrete plugin dialogs.
- Plugin dialog lifecycle and semantic signal wiring are centralized for
  consistent future UI styling and locale changes.
- Async plugin governance remains explicit in the coordinator.

### Limits

- Qt runtime plugin-dialog interaction, focus, visual hierarchy,
  accessibility, DPI, and screen-reader behavior remain unrun under the
  no-launch policy.
- Plugin policy, catalog validation, approval persistence, runtime enablement,
  and external execution invariants are unchanged and not re-proven by D23.
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

- Static source probe shows no PluginCatalogDialog/PluginStatusDialog import or
  direct construction in MainWindow and confirms all four callback routes.
- Compile, Ruff, format, handoff, package provenance, and release no-go checks
  are recorded in the D23 review and handoff.
- Qt startup and interactive plugin-dialog behavior remain unverified.
