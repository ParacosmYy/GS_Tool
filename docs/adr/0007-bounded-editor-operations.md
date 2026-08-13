# ADR 0007: Bounded, cooperative editor operations

- Status: accepted with limits
- Date: 2026-08-09
- Decision owners: Architect, Product, Developer 1, Developer 2, QA

## Context

QScintilla widgets must remain on the Qt UI thread. A synchronous Replace All therefore makes a large search and mutation occupy the event loop even when the operation has a match-count limit. Moving the widget to a worker would violate the editor adapter boundary and introduce unsafe cross-thread mutation.

## Decision

1. Keep matching and mutation inside the `EditorWidget` adapter.
2. Expose a `ReplaceAllSession` that has a counting phase followed by a replacement phase.
3. Let `MainWindow` own only orchestration: operation identity, tab ownership, status projection, cancellation, and Qt timer scheduling.
4. Inject `EditorOperationPolicy` from the application boundary. The current policy is 10,000 matches, an 8 ms slice budget, and at most 256 items per slice.
5. Perform no mutation before the counting phase confirms the match limit. A cancelled or failed replacement closes and rolls back its single undo transaction; if rollback itself fails, the shell reports that the document may contain partial changes instead of claiming restoration.
6. Keep the existing synchronous `EditorEngine.replace_all_literal` contract for non-shell callers; it drains the same session to completion.

## Consequences

- The UI can process user events between bounded slices without moving QScintilla across threads.
- A user can cancel a long replacement and receives an explicit status message.
- The limit protects correctness but does not establish a supported file-size range.
- Recovery text capture remains a separate measured risk because the editor text is copied on the UI thread before serialization. It is not silently treated as fixed by this ADR.
