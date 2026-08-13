# ADR-0023: Local session continuity

- **Status:** implementation in progress (D7.5.1)
- **Date:** 2026-08-09
- **Owners:** Architect, Product, Developer 1, Developer 2, QA

## User outcome

After a normal restart, QuillForge can restore the last local workspace root,
the order of path-backed document tabs, and the active tab. The restore is
offline, bounded, asynchronous, and failure-isolated. A missing or changed
file is reopened through the existing `DocumentService` path, so normal
encoding detection and external-change protection remain in force.

## Decision

Session continuity is a separate application contract from editor settings and
recovery. `SessionSnapshot` contains only a schema version, optional workspace
root, a bounded tuple of absolute document paths with a 0-based caret position,
and an active-document index. It never contains document text, undo history,
recovery payloads, plugin state, or editor widgets. Only clean path-backed tabs
are written; dirty and untitled content remains the responsibility of the D3
recovery flow.

`SessionService` owns normalization, duplicate removal, path/count bounds, and
safe defaults. `JsonSessionStore` owns only the bounded user-local JSON file at
`%LOCALAPPDATA%\\QuillForge\\session.json`, with a same-directory temporary
file, flush, `fsync`, and atomic replacement. Loading distinguishes an absent
file from a valid or invalid file through the typed `SessionLoadResult`
contract. A malformed, oversized, or unsupported session returns the empty safe
snapshot with `invalid` state and never overwrites the source file. Startup
and close preserve those original bytes until an explicit later session change
requests an atomic repair, as recorded in ADR-0026.

Startup uses a barrier owned by `MainWindow`: load the session asynchronously,
then scan and present recovery candidates, then restore the workspace and
session paths serially. Recovery is deliberately presented first so a restored
dirty tab wins over a clean session entry with the same path. Duplicate paths
are skipped without opening a second in-memory document. An explicitly closed
or cancelled recovery dialog is treated as Later, so its session path remains
deferred and cannot replace the recovery candidate. The blank initial tab
is created only after recovery and session restoration have finished and no tab
remains.

Session writes are latest-wins and single-flight through `TaskRunner`; tab,
workspace, and active-tab changes request a bounded metadata write. The close
guard observes the same retained worker/completion boundary, so the latest
queued session write cannot be abandoned while the window is destroyed.

## Scope and limits

In scope: one local profile; clean path-backed tabs; tab order; active tab;
workspace root; bounded versioned JSON; atomic local persistence; asynchronous
startup restore; recovery-first duplicate handling; missing/invalid path skip
with bounded status feedback; caret restoration through the editor adapter;
non-blocking close integration.

Out of scope for D7.5.1: document text in the session file; automatic dirty or
untitled restoration outside D3 recovery; selection/scroll state; undo history;
window geometry or dock layout; plugin runtime state; multi-instance
merge/locking; cloud or cross-device sync; encryption; crash-proof, hard-power,
clean-machine, cross-machine, or large-file support claims.

Absolute local paths are intentional for this first Windows-first profile, are
not written to logs by the session feature, and may become invalid after a
file move, permission change, machine change, or profile change. Invalid paths
are skipped and the remaining session continues.

## Verification

- Source/offscreen smoke proves manifest round-trip, duplicate/count/path
  normalization, corrupt/oversized fail-closed loading, atomic temporary-file
  cleanup, startup ordering, recovery-first Restore/cancel de-duplication,
  serial restore, missing-file continuation, active-tab/caret selection,
  latest-wins writes, and the close guard while a session write is pending.
- `scripts/check.ps1`, package/root identity, startup, and release handoff
  checks remain required after source changes.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are added or
  run by default.
