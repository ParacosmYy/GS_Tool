# D9 / UI-06 parent review

## Scope

- **Delivery:** D9 Modern UI iteration
- **Slice:** UI-06 TaskRunner-backed status coverage
- **Owner:** Architect (parent)
- **Change type:** presentation lifecycle projection with no application behavior change

## Product and architecture decision

The shell must not report READY while any retained background operation or its
queued completion callback remains active. `TaskRunner` is the existing shared
worker boundary, so its pending lifecycle is the narrowest source of truth for
recovery, session, search, workspace, settings, catalog, and plugin-host work.

## Implementation audit

- `TaskRunner.pending_changed` emits when a task is retained and after the
  queued completion callback releases it.
- `MainWindow._sync_status_rail()` gives WORKING priority to retained work,
  then projects dirty-document ATTENTION or idle READY.
- The existing `_busy` document-operation state, operation IDs, cancellation,
  and `closeEvent()` pending guard are unchanged.
- No worker receives a widget or status component; the signal and projection
  stay on the presentation/UI boundary.

## Verification and limits

- `scripts/check.ps1` is the required static and workflow gate.
- Package verification is required after the source change; the final artifact
  identity is recorded in the UI-06 handoff.
- QuillForge.exe startup, Qt-window inspection, and visual screenshots remain
  intentionally unrun because the current project instruction prohibits launch.

## Disposition

`accepted-with-limits`: the source contract closes the independent background
coverage gap; runtime visual, accessibility, DPI, and cross-machine evidence
remain open.
