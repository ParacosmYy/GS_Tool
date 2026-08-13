# ADR 0008: Cooperative recovery capture

- Status: accepted with limits
- Date: 2026-08-09
- Decision owners: Architect, Product, Developer 1, Developer 2, QA

## Context

Autosave previously called `EditorWidget.get_text()` in one UI-thread operation before submitting a recovery write. File I/O and JSON persistence were already worker-bound, but the initial copy could monopolize the event loop for a large document.

## Decision

1. Keep QScintilla reads on the UI thread; widgets are never moved to a worker.
2. Expose a `TextCaptureSession` from the editor adapter that reads position-aligned text chunks and yields after the injected time or chunk count.
3. Let `MainWindow` own document identity, content-version checks, stale-capture discard, timer continuation, and recovery lifecycle state.
4. Pass a tuple of already-read chunk references to `TaskRunner`; the chunk-capable recovery store JSON-encodes each chunk directly into the atomic temporary file instead of joining a second full string.
5. Use an 8 ms / 512-chunk / 16,384-character default capture policy. Scintilla positions, not Python string lengths, determine chunk boundaries. A single chunk is one adapter read and remains an explicit scale risk.
6. Preserve the existing snapshot ID, source revision, dirty-state, atomic-write, and cleanup semantics.

## Consequences

- Normal and long-line recovery capture yields to the event loop rather than making one full-text UI read.
- Editing during capture invalidates the stale capture instead of writing a mixed-version snapshot.
- The worker/store owns incremental JSON encoding; the UI still retains captured chunk strings until the worker consumes them, so native/process memory remains a measured risk.
- No large-file, native-memory, clean-machine, or hard-power-loss claim follows from this decision.
