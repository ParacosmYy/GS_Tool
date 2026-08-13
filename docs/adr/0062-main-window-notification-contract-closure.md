# ADR-0062: MainWindow notification contract closure

- **Status:** accepted-with-limits; D37a / UI-23 / ARCH-27 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D31 established an explicit `StatusMessageLevel` contract, and D34a–D36a
closed Session/Recovery, plugin/extension, and Workspace/operation domains.
Six remaining MainWindow call sites still relied on the default `info` level:
new-document completion, session-restore fallback, stale command, settings
availability/in-flight guards, and active-tab projection. This left a hidden
default in the coordinator even though the presentation contract supported
semantic levels.

## Decision

Make every MainWindow `notify` call pass an explicit legal level while keeping
the public compatibility path unchanged:

- new document -> `success`;
- invalid/skipped session document, stale command, and settings save already
  in progress -> `warning`;
- settings persistence unavailable -> `error`;
- active document/tab projection -> explicit `info`.

The existing D34a–D36a mappings remain unchanged. The `notify(message)` API
still defaults to `info` for external plugin/application callers that do not
need metadata; this slice only closes the MainWindow call-site contract.

## Invariants

1. No DocumentService, SessionService, SettingsService, TaskRunner, plugin
   callback, operation ID, close guard, locale route, or state machine moves.
2. No notification text is parsed to infer severity.
3. StatusSurface remains the presentation owner; MainWindow remains the
   outcome/policy owner.
4. The change adds no new state, service, event, or dependency direction.

## Alternatives considered

- **Keep defaults at remaining call sites:** rejected because a coordinator
  contract should not depend on an implicit visual state.
- **Infer the level inside `notify`:** rejected because localization and
  future copy changes would become policy inputs.
- **Introduce a notification policy service:** rejected because all outcomes
  are local to the existing coordinator and no second consumer exists.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code, not embedded C/C++ or
firmware. MCU/vendor requirements are not applicable. Public CloudWeGo pages
remain transferable engineering references only; they do not establish a
private ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static AST probe proves all 81 MainWindow `notify` calls have an explicit
  legal transient level (`info`/`success`/`warning`/`error`) and the
  one-argument API remains available; permanent operation progress stays in
  the separate `WORKING` phase projection.
- Compile, Ruff, format, independent review, handoff, package provenance, and
  release no-go evidence are recorded.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Qt rendering, interaction, screen-reader output, font metrics, DPI,
cross-machine appearance, clean-machine evidence, and release-owner gates
remain unrun. This closure does not refactor MainWindow into coordinators or
claim runtime visual acceptance.
