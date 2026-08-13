# ADR-0067: workspace action hierarchy

- **Status:** accepted-with-limits; D42 / UI-28 bounded visual slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace panel already had stable object names for its three actions:
opening a workspace, navigating to the parent directory, and cancelling an
active load. They all inherited the same generic `QPushButton` treatment,
which weakened the visual hierarchy and made the secondary actions look like
peer commands. The user-visible issue is presentation-only; the existing
signals and loading policy are already correct.

## Decision

Keep the centralized theme token/QSS boundary and add explicit states for the
existing `workspaceBack` and `workspaceCancel` object names. The open-folder
button remains the primary gradient action. Back becomes a quiet outlined
secondary action with accent-alt hover/focus, while cancel remains quiet until
hover/focus and then uses the existing warning/gold token. All three controls
retain their existing signals, enabled state, and object names.

The slice does not add a component, theme token, animation engine, signal,
application service, or policy owner. It only narrows selector specificity in
`presentation.theme._stylesheet` so the shared visual system owns the states.

## Invariants

1. `workspaceBack`, `workspaceCancel`, and `primaryAction` object names remain
   unchanged.
2. No signal connection, label, locale key, loading guard, or workspace policy
   changes.
3. Back and cancel expose hover, focus, pressed, and disabled states; the
   cancel hover/focus foreground uses the same theme's warning/gold endpoint.
4. The existing primary-action readable foreground calculation remains the
   only accent endpoint foreground selector; no hard-coded per-theme override
   is introduced.
5. The QSS remains centralized in `theme.py`; no second styling system is
   introduced.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source probe proves both existing object-name selectors and all four
  interaction states, including disabled and focus coverage.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence
  are recorded.
- Independent review records selector specificity, theme contrast risk, and
  behavior preservation.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits and simplification

The smallest safe change is two selector groups using existing object names
and tokens; a new action component or token layer would increase coupling
without solving a new problem. Native QSS rendering, font metrics, DPI,
platform palette behavior, and visual acceptance remain unrun under the
no-launch policy. This is a bounded action-hierarchy refinement, not a full
visual redesign or release approval.
