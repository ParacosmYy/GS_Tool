# ADR-0059: Session and Recovery notification severity closure

- **Status:** accepted-with-limits; D34a / UI-20 bounded coordinator slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D31 introduced explicit `info`/`success`/`warning`/`error` levels on the
presentation notification seam. D32a and D33 extended visible state hierarchy
to inline workspace/search/FindBar surfaces, but Session and Recovery outcomes
in MainWindow still used the default info level even for failures, fallback,
cancellation-adjacent warnings, and successful restore/discard operations.
Users therefore saw text differences without a consistent severity signal.

## Decision

Close only the existing Session/Recovery notification call-site gap. Add
explicit levels to the 19 targeted `MainWindow` notifications while retaining
the existing `notify(message)` API and `StatusSurface` implementation:

- autosave/capture/save/scan/cleanup failures and invalid persistence results
  -> `error`;
- unavailable, already-in-progress, deferred, postponed, and kept/fallback
  decisions -> `warning` where user attention is required;
- successful recovery and asynchronous snapshot discard -> `success`;
- neutral “no snapshots” and “newer edits will be captured next cycle” status
  -> explicit `info`.

This is a presentation metadata change only. RecoveryService, SessionService,
TaskRunner, operation IDs, stale guards, snapshot lifecycle, close guards,
session ordering, and document policy remain in their existing owners.

## Invariants

1. The one-argument `notify(message)` compatibility path remains unchanged.
2. No notification string is parsed to infer severity.
3. No recovery/session state machine, callback ordering, persistence payload,
   cancellation path, or worker behavior changes.
4. MainWindow remains the owner of use-case outcome interpretation; StatusSurface
   remains the owner of notification presentation.
5. All levels are members of the existing `StatusMessageLevel` contract.

## Alternatives considered

- **Leave all calls at info:** rejected because recoverable failures and
  successful outcomes remain visually indistinguishable.
- **Infer levels inside `notify()` from text:** rejected because it couples
  policy to English/localized copy.
- **Create a RecoveryNotificationPolicy service:** rejected for this bounded
  slice; it would move stable coordinator interpretation without a typed
  application contract or a demonstrated need.

## Limits

Qt startup, native rendering, screen-reader output, font metrics, DPI,
cross-machine appearance, clean-machine evidence, and release-owner gates
remain unrun under the no-launch policy. This slice does not close the remaining
plugin/document/workspace notification call-site audit.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation/coordinator code, not embedded C/C++ or
firmware. Embedded MCU/vendor requirements are N/A. Public CloudWeGo sources
remain transferable engineering references only, not private ByteDance
standards or certification:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static AST probe covers the 19 Session/Recovery notification calls and proves
  each has a legal explicit level.
- Compile, Ruff, format, independent review, handoff, package provenance, and
  release no-go evidence are recorded in the D34a handoff.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run.
