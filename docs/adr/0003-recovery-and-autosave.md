# ADR 0003: Explicit recovery snapshots for unsaved documents

- **Status:** accepted for D3 MVP
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The atomic `.bak` created by normal saves protects the previous disk version, but it cannot recover text that was never saved. QuillForge must protect unsaved work without making autosave look like a normal save or exposing editor widgets to infrastructure code. A full `editor.get_text()` call also creates an unbounded UI-thread copy for large documents.

## Decision

1. A dirty tab may produce one latest recovery snapshot in the per-user `QuillForge/recovery` directory. The first MVP uses a fixed 30-second timer; settings belong to D5.
2. A snapshot is a versioned JSON envelope containing document ID, optional original path, text, encoding, line ending, source disk revision, and a nanosecond timestamp. It is written to a same-directory temporary file, flushed and fsynced, then atomically replaced.
3. The presentation layer advances an adapter-owned position-safe text capture session in bounded UI slices, then submits worker-side chunk joining, snapshot creation, and persistence to the existing `TaskRunner`. It never passes `EditorWidget` to a worker.
4. Startup recovery is opt-in per candidate. The user sees the source path, timestamp, source status, and the consequences of Restore, Discard, or Later. Restore keeps the document dirty and retains the source revision so normal external-change protection remains active.
5. A snapshot is removed only after an explicit discard or a successful normal save. Failed snapshot writes do not clear dirty state and do not delete the previous valid snapshot. If the content version changes during capture, the stale capture is discarded and a later cycle may retry.

## Rejected alternatives

- Reusing `.bak`: it contains only an earlier disk version and cannot represent unsaved editor text.
- Automatically overwriting the original on recovery: it would turn an explicit recovery choice into an unsafe save and could destroy externally changed bytes.
- A settings UI in D3: the fixed interval is intentionally temporary; generalized settings are D5.
- A second thread pool: the existing worker boundary is sufficient and keeps concurrency policy centralized.

## Limits and follow-up

This MVP retains only the latest valid snapshot per document, does not restore cursor/selection/undo history, and does not claim large-file, power-loss, encrypted, or clean-machine evidence. A single large chunk can still exceed the nominal capture slice because it is one adapter read. D5 owns configuration; D7 owns measured scale and latency work; D8 owns enterprise release and support evidence.
