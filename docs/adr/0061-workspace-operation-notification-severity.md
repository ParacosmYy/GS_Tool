# ADR-0061: Workspace and operation notification severity

- **Status:** accepted-with-limits; D36a / UI-22 / ARCH-26 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The shared status-message hierarchy now distinguishes semantic outcomes, but
workspace navigation and Find in Files still left several shell notifications
at the default informational level. The same was true of the coordinator's
generic long-running operation message, even though the status rail already
projected `WORKING`. Startup restore guards also appeared as neutral text while
blocking user actions.

## Decision

Reuse the existing `StatusMessageLevel` contract at the current MainWindow
boundary:

- `_begin_operation()` keeps the permanent StatusRail in `working` and
  projects its transient copy as explicit `info`, because the existing
  StatusMessageLevel contract intentionally has four levels;
- workspace/search service unavailable, invalid result, and operation failure
  outcomes are `error`;
- startup restore guards, missing workspace roots, containment rejections,
  cancellation, no-match search results, truncation/limit outcomes, and
  bounded search diagnostics are `warning`;
- clean workspace-open and complete search results are `success`.

The search summary level is selected from the typed `WorkspaceSearchResult`
fields (`cancelled`, `truncated`, `limit_reason`, `issues`, and matches), not
from localized text. An invalid search result now reaches both the existing
inline error projection and the shell error channel.

## Invariants

1. Workspace generation checks, TaskRunner operation IDs, cooperative
   cancellation, root containment, session-restore barriers, and document-file
   open policy remain unchanged.
2. `MainWindow` retains application outcome interpretation; WorkspaceSurface,
   WorkspaceSearchSurface, and StatusSurface retain presentation projection.
3. The existing one-argument `notify(message)` compatibility path remains
   valid for callers outside the slice.
4. No new state model, service, policy parser, or widget-to-application
   dependency is introduced.

## Alternatives considered

- **Infer severity from summary/message text:** rejected because localization
  and wording would become policy inputs.
- **Create a WorkspaceNotificationPolicy service:** rejected for this bounded
  slice because one coordinator remains the only consumer and the typed result
  fields already provide a stable contract.
- **Change inline widget state instead of the shell:** rejected because the
  inline surfaces already carry their own D32a state; this slice closes the
  remaining transient shell projection.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code, not embedded C/C++ or
firmware. MCU/vendor requirements are not applicable. Public CloudWeGo pages
remain transferable engineering references only; they do not establish a
private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static AST/source probes prove legal explicit levels for Workspace/search
  notifications, all startup restore guards, and operation progress.
- Compile, Ruff, format, independent review, handoff, package provenance, and
  release no-go evidence are recorded.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Qt rendering, interactive workspace/search/file flows, screen-reader output,
font metrics, DPI, cross-machine appearance, clean-machine evidence, and
release-owner gates remain unrun. This slice does not extract MainWindow or
classify unrelated settings/document/session copy outside the addressed
workspace/operation boundary.
