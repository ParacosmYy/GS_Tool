# ADR-0058: FindBar semantic feedback projection

- **Status:** accepted-with-limits; D33 / UI-19 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

D32a gave WorkspacePanel and WorkspaceSearchDialog explicit inline feedback
states, but FindBar still rendered every editor-operation result through one
muted `set_status(message)` path. FindBar has short synchronous find/replace
outcomes as well as a cooperative Replace All lifecycle, so the mapping must
be explicit at MainWindow call sites without moving editor or operation policy
into the widget.

## Decision

Extend the existing FindBar/FindSurface status seam with an optional keyword
`level: FeedbackLevel = "info"`. The one-argument `set_status(message)` call
remains valid. FindBar stores the level beside the raw message, reapplies it
when locale changes, and delegates property/repolish to the shared
`presentation.feedback.apply_feedback_state` helper. Theme QSS adds the same
five-state hierarchy to `findStatus`.

MainWindow supplies the semantic mapping at existing call sites:

- successful match and replacement -> `success`;
- no match, empty query, stale selection, busy conflict, policy/preflight
  rejection, match-limit stop, and cancellation -> `warning`;
- Replace All counting/progress -> `working`;
- Replace All failure or partial-change failure -> `error`.

FindSurface remains a thin composition boundary. It forwards the typed level;
it does not inspect text or editor state. MainWindow retains active-tab lookup,
literal editor calls, Replace All session/cancellation/rollback, operation IDs,
tab locking, notifications, and error policy.

## Invariants

1. Existing one-argument FindBar/FindSurface status calls remain source- and
   behavior-compatible.
2. Feedback level is presentation metadata and is never inferred from
   localized message text.
3. Locale refresh preserves the current raw message and current level.
4. Operation-active/reset/close behavior and all existing signal payloads stay
   unchanged.
5. FindBar imports only presentation feedback/i18n/domain locale contracts;
   it does not import MainWindow, application services, editor adapters, or
   filesystem policy.

## Alternatives considered

- **Keep text-only status:** rejected because the user cannot scan outcome
  hierarchy reliably and it duplicates the D32a visual gap.
- **Infer level from message text:** rejected because localization and future
  copy changes would silently alter semantics.
- **Move mapping into FindBar:** rejected because editor outcomes and
  Replace All lifecycle belong to MainWindow's application/presentation
  coordinator boundary.

## Limits

Qt startup, native QSS specificity, interactive find/replace, animation,
screen-reader output, font metrics, DPI, and cross-machine appearance remain
unrun under the no-launch policy. D33 covers FindBar only; later UI slices may
still refine copy, spacing, and full runtime visual acceptance.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware. The
embedded-enterprise-workflow and embedded-code-review-simplifier are N/A for
MCU/vendor constraints. Public CloudWeGo sources are transferable engineering
references only, not private ByteDance standards or certification:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static AST/source probe proves the compatibility default, thin FindSurface
  forwarding, 17 MainWindow semantic calls, and all five QSS selectors.
- A targeted QSS contrast probe covers 60 pairs across every supported
  theme/accent combination at >= 4.5:1.
- Compile, Ruff, format, independent review, handoff, package provenance, and
  release no-go evidence are recorded in the D33 handoff.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run.
