# ADR-0066: workspace-entry activation boundary

- **Status:** accepted-with-limits; D41 / ARCH-31 / UI-27 corrected bounded interaction slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace tree already exposes two mouse-specific routes: a single click
opens a file and a double click enters a directory. Keyboard activation through
the focused tree item is not routed to either semantic callback. This makes
file opening appear inconsistent and leaves a keyboard-first editor without a
complete activation path.

The widget must remain a presentation projection. MainWindow owns workspace
containment, startup/busy guards, asynchronous document loading, and all error
and notification policy. A first implementation used Qt's `itemActivated`
signal, but independent review identified that Qt may emit it for mouse
activation depending on the active style, which could duplicate or misroute the
existing mouse contract.

## Decision

Keep the existing mouse behavior and add a small `_WorkspaceTree` presentation
subclass that intercepts only Enter/Return key presses. It emits a dedicated
keyboard activation signal for the current non-null item; `WorkspacePanel`
routes that signal through one small helper to `file_requested` or
`directory_requested`. Existing `itemClicked` and `itemDoubleClicked` handlers
remain the only mouse routes for the established single-click-file /
double-click-folder behavior. Nullable Qt item inputs are ignored before any
item data is read.

No new application service, path policy, document coordinator, or widget is
introduced. MainWindow remains the only owner of containment checks and the
async open/list operations.

## Invariants

1. File entries still emit `file_requested` on the existing mouse click path.
2. Directory entries still emit `directory_requested` on the existing mouse
   double-click path.
3. Enter/Return keyboard activation emits exactly the semantic route matching
   the entry kind and never opens inaccessible entries.
4. Mouse activation remains independent of the keyboard route; the
   style-dependent `itemActivated` signal is not used.
5. Null item pointers from Qt callbacks are safe no-ops.
6. The emitted path remains the immutable item payload; no filesystem call is
   made by the widget.
7. MainWindow continues to own workspace containment, startup/busy guards,
   TaskRunner, document loading, notifications, and stale-operation policy.

## Alternatives considered

- **Leave the mouse-only routes unchanged:** rejected because keyboard
  activation remains a real user-visible gap and makes file opening feel
  unreliable.
- **Use Qt `itemActivated`:** rejected because its mouse-versus-keyboard
  delivery is style-dependent and can duplicate the established mouse routes.
- **Move file opening into WorkspacePanel:** rejected because it would leak
  filesystem/document policy into a Qt widget.
- **Replace mouse behavior with activation-only behavior:** rejected because
  it would change the accepted single-click-file interaction unnecessarily.
- **Add a new workspace application service:** rejected because no new use
  case or path-policy decision is required for this bounded projection fix.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source probe proves Enter/Return-only keyboard routing, null safety,
  matching file/directory callbacks, and unchanged mouse routes.
- Static inspection proves no filesystem, document service, TaskRunner, or
  containment code enters `WorkspacePanel`.
- Compile, Ruff, format, handoff, package provenance, and release no-go
  evidence are recorded.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits

Native Qt event ordering, mouse/keyboard interaction, focus traversal,
accessibility, DPI, font metrics, cross-machine appearance, and release-owner
gates remain unrun. This is an activation-route correction, not runtime visual
acceptance or a complete document coordinator extraction.
