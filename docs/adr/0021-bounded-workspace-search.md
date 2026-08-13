# ADR-0021: Bounded asynchronous workspace search

- **Status:** implementation in progress (D7.4)
- **Date:** 2026-08-09
- **Owners:** Architect, Product, Developer 1, Developer 2, QA

## User outcome

After opening a workspace, an operator can use **Edit → Find in Files** (`Ctrl+Shift+F`) to find literal text across nested local files. Results show a relative path, one-based line/column, and a bounded preview. Double-clicking a result opens the file asynchronously and positions the editor at the reported line.

## Decision

Add a dedicated workspace-search capability with this dependency direction:

```text
WorkspaceSearchDialog ──> MainWindow ──> TaskRunner
                                      └──> WorkspaceSearchService
                                              └──> WorkspaceSearchProvider
                                                      └──> local filesystem
```

The existing `WorkspaceProvider` remains a one-directory navigation port. Search does not reuse `WorkspacePanel`, `DocumentStore`, `FindBar`, QScintilla controls, or plugin callbacks.

`WorkspaceSearchService` validates an explicit resolved root and non-empty literal query, then delegates an immutable `WorkspaceSearchRequest` containing the query and a `WorkspaceSearchPolicy`. The infrastructure adapter walks regular non-symlink entries in deterministic case-folded path order. It supports UTF-8/UTF-16 BOM text, skips NUL-detected binary files, and reads line-by-line with a bounded line length. Search is read-only and returns immutable matches, bounded issues, counters, and explicit `cancelled`/`truncated` state.

The optional cancellation callback is strict at the application boundary:
`None` selects an explicit no-cancellation callback, while every non-`None`
value must be callable. Falsey non-callable values are rejected rather than
silently treated as the default.

If the bounded issue ledger fills, the result is marked limited with
`max-issue-records`; dropped diagnostics are never presented as a complete
scan. Provider matches and issue paths are validated against the resolved root
before presentation.

The filesystem adapter also enforces the byte budget during the read, not only
from the initial `stat().st_size` admission. A streaming raw-reader cap sits
below the text decoder, distinguishes exact EOF from a file that grows after
admission, and accounts actual bytes against the remaining total budget. A
zero-byte remaining budget is valid for an empty file and does not consume body
bytes.

## Configured safety policy

The acceptance contract is the source of the product bounds:

| Bound | Current value |
|---|---:|
| files considered | 2,000 |
| total bytes admitted | 64 MiB |
| bytes per file | 4 MiB |
| entries considered per directory | 4,096 |
| matches | 10,000 |
| recursive depth | 32 |
| line bytes | 256 KiB |
| preview characters | 180 |
| diagnostic records | 32 |

The default excluded directory names are `.git`, `.venv`, `__pycache__`, `.pytest_cache`, `build`, `dist`, and `node_modules`. These values bound work and do not constitute a measured support range, performance SLA, native-memory ceiling, or cross-machine guarantee.

## Cancellation and stale results

The presentation layer captures a `threading.Event` in the worker operation. The provider checks it before directories, files, and lines and returns a cancelled result with matches collected so far. `MainWindow` retains the operation ID and generation; only the current completion may update the dialog. Closing the main window requests cancellation and waits for the worker completion guard before accepting shutdown. TaskRunner remains a scheduling/marshalling boundary and does not own search policy.

## Result activation safety

The result contains a path only as application data. Before opening it, `MainWindow` asks `WorkspaceService.contains()` against the currently active root. The document is loaded through the existing asynchronous document service and the editor port receives only the one-based line navigation request. No result opens a file outside the active workspace or passes a widget to the worker.

## D7.4.x diagnostic projection

The search result distinguishes ordinary bounded `issues` from an
`issues_truncated` flag. The provider sets the flag when the bounded diagnostic
ledger is full, even if another safety limit was reached first; this prevents a
secondary diagnostic overflow from being hidden by the first limit reason.

`WorkspaceSearchDialog` projects the bounded issue records through a collapsed,
read-only diagnostics section. Each record exposes only a path relative to the
active workspace and a bounded reason. The section is hidden when there are no
diagnostics, and its header explicitly says when additional diagnostics were
omitted. It owns no filesystem access and does not change result activation,
cancellation, stale-result, or containment behavior.

Preview projection expands tabs before measuring and slicing so the displayed
width, including leading/trailing truncation markers, never exceeds
`max_preview_chars`. Marker space is reserved before the body is sliced; this
keeps a trailing marker visible when tab expansion makes the rendered line
longer than its source-character length. This is a presentation projection
only and does not change match offsets or search semantics.

This is a diagnosability increment, not a new search capability. It does not add
regex, indexing, cross-file replacement, remote roots, or a performance SLA.

The Qt-free packaged diagnostic report also records `execution` and an
`artifact` identity. Frozen reports include the producing EXE's resolved path,
size, and SHA-256; source-Python reports use `artifact: null`.

## Scope and limits

In scope: literal, line-local matching; deterministic traversal; bounded recursive local files; UTF-8/UTF-16 BOM decoding; binary/symlink/excluded-directory handling; bounded diagnostics; cancellation; stale-result protection; line navigation.

Out of scope: regex, whole-word semantics, indexing, remote/network roots, archive contents, cross-file replacement, plugin-provided search, file watching, and general large-file support claims. Long-line and oversized-file skips are visible through limited/diagnostic state.

## Verification

- Inline source smoke uses a temporary real workspace to cover deterministic matches, UTF-8/UTF-16, binary/excluded/symlink boundaries, bounds, long lines, and cancellation.
- Qt offscreen smoke constructs the real dialog, projects a result, confirms bounded status text and result activation data, and verifies busy/cancel state.
- Qt offscreen smoke also verifies relative diagnostic projection, collapsed-by-default behavior, and explicit diagnostic-ledger truncation.
- The packaged root EXE exposes an explicit Qt-free `--diagnose-workspace-search <root> <literal> --report <path> [--case-sensitive]` probe; it writes bounded JSON diagnostics and does not enable a general command-line search service.
- `scripts/check.ps1`, package, root/dist hash synchronization, startup, repeat, measure, clean-machine preflight, and release handoff checks remain required.
- Independent after-source and before-release review outcomes are recorded separately; unavailable child review is not claimed as a pass.
- The D7.4 cancellation callback repair has a post-fix independent Luna source
  review and a falsey non-callable input probe; packaged/runtime evidence is
  still required separately.
- The scan-time byte-budget follow-up has a parent architecture review and an
  independent review record; its remaining runtime/package limits are explicit.
