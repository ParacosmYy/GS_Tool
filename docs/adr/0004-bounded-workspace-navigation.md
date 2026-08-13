# ADR 0004: Explicit, bounded workspace navigation

- **Status:** accepted for D4 MVP
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The editor needs project context without turning a user-selected folder into an unbounded filesystem scan. Recursive indexing, workspace persistence, and cross-file search depend on later performance and settings decisions.

## Decision

1. The user explicitly chooses a root folder. `WorkspaceService` accepts only paths resolved inside the active root for subsequent navigation.
2. `FileWorkspaceProvider` enumerates one directory at a time with a hard limit of 500 entries. It does not recurse and does not follow child symbolic links. Inaccessible or unsupported entries remain visible as non-activatable status rows when possible.
3. Directory enumeration and document opening run through the existing `TaskRunner`. `WorkspacePanel` receives immutable domain projections and emits semantic intents; it does not expose `QFileSystemModel` or filesystem traversal to application code.
4. A workspace root is activated only after its bounded page succeeds on the UI thread. Cancelled or failed loads keep the previous root and visible page. Generation and operation checks discard stale results.
5. Opening a workspace file reuses `DocumentService` and the existing duplicate-path and external-change protections. Workspace navigation never writes, renames, deletes, or modifies files.

## Rejected alternatives

- Recursive scan or project index: unbounded work and performance claims belong to D7.
- `QFileSystemModel` as the application contract: it would leak Qt and traversal policy across the boundary.
- Automatic root activation in the worker: cancellation could leave the UI and service root inconsistent.
- File-system mutations from the navigator: those require separate product and permission decisions.

## Limits and follow-up

The MVP does not persist workspace roots, filter binary files by content, support file mutations, or provide a true interrupt of an already-running OS directory call. The Cancel action prevents its result from being applied; enumeration is bounded to limit the exposure window. D5 owns workspace/settings persistence and D7 owns measured indexing and large-directory performance.
