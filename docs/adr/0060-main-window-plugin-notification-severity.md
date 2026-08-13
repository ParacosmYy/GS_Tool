# ADR-0060: MainWindow plugin and extension notification severity

- **Status:** accepted-with-limits; D35a / UI-21 / ARCH-25 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D31 introduced explicit notification levels at the existing `StatusSurface`
boundary. D34a closed the Session/Recovery call-site gap, but the plugin and
extension coordinator still presented unavailable services, asynchronous
progress, successful mutations, and failures through the default informational
level. A bounded source review also found two D34a classifications that needed
correction: invalid session persistence is an error, while a deferred recovery
snapshot requires user attention and is a warning.

## Decision

Keep outcome interpretation in `MainWindow` and add explicit levels to the
existing plugin/extension notification calls:

- catalog/runtime/host/governance unavailable, invalid results, operation
  failures, and host failures -> `error`;
- duplicate/in-flight operations and a host policy rejection -> `warning`;
- catalog scan, host probe, and governance mutation in progress -> explicit
  `info`; the existing TaskRunner pending projection keeps the permanent
  StatusRail in `WORKING`;
- clean catalog completion, ready host probe, plugin enable/disable, and
  governance completion -> `success`;
- a catalog truncated or carrying scan/approval diagnostics -> `warning`.

Also correct the two existing Session/Recovery classifications in the same
coordinator contract: invalid or failed session persistence is `error`, and a
recovery snapshot kept for later review is `warning`.

The change reuses `StatusMessageLevel` and the existing `notify` compatibility
path. It does not add a policy service, parse localized text, introduce a
second state model, change async operation IDs, alter stale-result guards, or
load external code.

## Invariants

1. The one-argument `notify(message)` path remains valid for plugin callbacks
   and other existing callers.
2. `MainWindow` remains the application outcome/policy owner; `StatusSurface`
   remains the presentation owner.
3. Plugin catalog trust, approval, runtime enablement, host containment, and
   execution-disabled invariants remain in their existing application and
   infrastructure owners.
4. Stale completion guards, worker submission, governance action locking, and
   command-menu refresh behavior are unchanged.

## Alternatives considered

- **Infer levels from notification text:** rejected because it couples policy
  to localized copy and makes future wording changes unsafe.
- **Create a plugin notification policy service:** rejected for this bounded
  slice because it would move stable coordinator interpretation without a
  demonstrated cross-window consumer or typed contract need.
- **Use one success level for every completed scan:** rejected because
  truncation and typed scan/approval diagnostics require user attention.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code, not embedded C/C++ or
firmware. MCU/vendor requirements are not applicable. Public CloudWeGo pages
remain transferable engineering references only; they do not establish a
private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static AST/source probes prove explicit legal levels across the targeted
  plugin/extension coordinator methods and the corrected D34a branches.
- Compile, Ruff, format, independent review, handoff, package provenance, and
  release no-go evidence are recorded for the slice.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Qt rendering, interaction, screen-reader output, font metrics, DPI,
cross-machine appearance, clean-machine evidence, and release-owner gates
remain unrun. This slice does not extract `MainWindow` or close unrelated
document/workspace notification calls.
