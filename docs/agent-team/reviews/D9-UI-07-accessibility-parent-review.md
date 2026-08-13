# D9 / UI-07 parent review — static accessibility baseline

- **Date:** 2026-08-09
- **Delivery:** D9 Modern UI iteration
- **Slice:** UI-07 static accessibility baseline
- **Owner:** Architect (parent)
- **Disposition:** ACCEPTED WITH LIMITS; source/style baseline integrated while
  runtime visual and assistive-technology evidence remain open

## Trigger

A read-only Luna Product/Developer/QA audit found that the modal error helper
set the status rail to ERROR without restoring the normal lifecycle phase after
the dialog returned. The same audit found that shared list/tree focus was
explicitly suppressed and that the muted text token was weak against the ink
surfaces. The audit did not find an icon-resource or TaskRunner projection
break.

## Fix audit

- `_show_error()` now calls `_sync_status_rail()` after `QMessageBox.critical`
  returns, so the normal WORKING/ATTENTION/READY projection is authoritative
  again.
- `StatusRail` has stable accessible names and a phase description that tracks
  the explicit phase label.
- `theme.py` centralizes cyan focus borders for common controls, tabs, and
  workspace/search lists, removes the list/tree `outline: 0` suppression, and
  raises `TEXT_MUTED` while retaining the ink/violet palette.
- No application ports, worker boundaries, command ownership, or close guards
  changed.

## Review status and limits

- The original Luna audit supplied actionable findings. A later Boyle/Luna
  post-fix static audit returned PASS and is recorded in
  `D9-UI-07-independent-luna-follow-up.md`; runtime acceptance remains open.
- The parent reviewed the source and owns the integration decision.
- Runtime Qt startup, screenshots, DPI, native focus, contrast measurement,
  and screen-reader behavior remain unrun because the project instruction
  prohibits launching QuillForge.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were added or
  run.
