# ADR-0070: workspace file activation consistency

- **Status:** accepted-with-limits; D45 / ARCH-35 / UI-31 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace tree already emitted a semantic file intent on a single click
and a directory intent on a double-click. That contract was technically
valid, but it was easy to misread as “folders open, files do not” because a
double-click on a file was intentionally ignored. The same path could also
be requested twice by a fast click/double-click sequence.

## Decision

Keep activation semantics in the existing presentation boundary and make a
double-click activate either visible entry kind. Preserve single-click file
opening and Enter/Return activation. MainWindow remains the policy owner: it
rejects activation during an active operation, rechecks workspace containment,
reuses an already-open tab, and otherwise dispatches through the existing
asynchronous `DocumentService.open_document` path.

No new coordinator, service, filesystem call, or Qt signal is introduced.

## Invariants

1. Files can be activated by single click, double-click, or Enter/Return.
2. Directories can be activated by double-click or Enter/Return.
3. Inaccessible entries remain inert.
4. A busy MainWindow does not start or switch a workspace-open operation.
5. Workspace containment and async document dispatch remain in MainWindow.
6. Re-activating a path already represented by a tab selects that tab instead
   of creating a duplicate open request.

## Alternatives considered

- **Keep double-click files inert:** rejected because it conflicts with the
  normal file-tree interaction model and caused the reported perception.
- **Move opening policy into WorkspacePanel:** rejected because it would leak
  containment, tab identity, busy gating, and document dispatch into Qt code.
- **Add a new activation coordinator:** rejected because the existing
  WorkspaceSurface semantic callbacks already form the correct boundary; the
  smallest complete fix is local to the panel and MainWindow guard.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

## Verification target

- A source probe proves click, double-click, keyboard, busy, containment, and
  existing-tab branches.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence
  are recorded.
- Independent review records a conclusion or an explicit no-conclusion state.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits and simplification

The smallest safe change is one presentation route adjustment plus one
MainWindow guard. A timer-based double-click debounce was not added because
the existing busy gate and existing-tab reuse make the visible behavior
idempotent without introducing another lifecycle owner. Native event ordering,
thread timing, runtime navigation, visual rendering, accessibility,
cross-machine behavior, and release-owner gates remain unrun.
