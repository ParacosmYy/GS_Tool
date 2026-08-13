# ADR-0056: Semantic transient notification hierarchy

- **Status:** accepted-with-limits; D31 / UI-17 bounded presentation slice
- **Date:** 2026-08-10
- **Decision owner:** Architect

## Context

The status surface already owns the status-bar host, the permanent lifecycle
rail, locale projection, and transient notifications. Its notification API
currently projects every message as unstyled native status-bar text. That makes
ordinary progress, successful outcomes, recoverable warnings, and errors look
the same, even though the shell already exposes distinct semantic colors and
phase states. The lack of hierarchy is especially visible in the compact
command/workspace shell.

## Decision

Keep `StatusSurface` as the only presentation boundary for transient shell
notifications. Replace the native temporary-message projection with one
localized, theme-styled `statusMessage` label that supports four presentation
levels: `info`, `success`, `warning`, and `error`.

`MainWindow.notify(message)` remains source-compatible for plugin and existing
callers; it gains an optional presentation-only `level` keyword defaulting to
`info`. Only clear outcome/error call sites opt into a non-default level. The
surface owns message lifetime, timeout behavior, localization, state-property
refresh, and status-bar insertion. MainWindow retains operation policy,
notification text, and all application decisions.

## Invariants

1. Existing one-argument notification calls remain valid and render as `info`.
2. `StatusSurface` remains Qt/domain/i18n-only; it imports no application
   service, document state, persistence adapter, worker, or operation policy.
3. The default transient lifetime remains 5000 ms; a zero timeout remains
   persistent until the next message or explicit clear, and negative values
   clamp to zero as before.
4. Locale changes retranslate the current message through the same surface
   seam; the raw diagnostic detail is retained for later locale refresh.
5. The four state styles use existing centralized theme tokens. No widget-local
   stylesheet or second color source is introduced.
6. No command callback, shortcut, file-open behavior, workspace containment,
   plugin capability, task lifecycle, or status-phase precedence changes.

## Alternatives considered

- **Keep native `QStatusBar.showMessage()` only:** preserves the old projection
  but cannot reliably express per-message semantic emphasis through the shared
  QSS contract.
- **Add a global notification bus:** would broaden coupling and move policy
  away from the existing `StatusSurface` seam; rejected.
- **Infer severity from arbitrary message strings:** would turn presentation
  into a fragile parser; explicit opt-in levels are safer and source-compatible.

## Limits

Qt startup, status-bar rendering, timeout timing, native-style metrics, DPI,
font availability, accessibility/screen-reader output, and cross-machine
appearance remain unrun under the current no-launch policy. This slice does
not claim release readiness or private ByteDance engineering compliance.

## Public-source applicability

The project is a Python/PyQt6 desktop application, not embedded C/C++ or
firmware. The public CloudWeGo references in the enterprise architecture
baseline are transferable engineering references only; they are not
manufacturer requirements, private ByteDance standards, or certification
evidence:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- Static source review proves the surface owns the notification widget,
  localization, level property, timeout, and status-bar placement.
- A static theme probe proves all four state selectors use centralized tokens.
- Compile, Ruff, format, handoff, package identity, and release no-go checks
  are recorded in the D31 handoff.
- No unit tests, Qt launch, screenshots, deployment, or hardware operation are
  created or run.
