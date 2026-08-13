# D41 independent review — workspace-entry activation

- **Delivery:** D41 / ARCH-31 / UI-27
- **Reviewers:** Faraday the 2nd / Luna max (initial), Turing the 2nd / Luna max (post-correction)
- **Review mode:** read-only; no files edited and no worktree used
- **Initial verdict:** FAIL — do not accept as-is
- **Post-correction status:** PASS with runtime limits

## Findings

The initial D41 implementation connected `itemClicked`,
`itemDoubleClicked`, and Qt's `itemActivated` independently. Qt documents
`itemActivated` as style-dependent mouse activation as well as keyboard
activation, so the route could duplicate mouse delivery or violate the
single-click-file / double-click-folder contract. The official contract is
documented at <https://doc.qt.io/qt-6/qtreewidget.html#itemActivated>.

The initial `_item_path_and_kind()` path dereferenced a nullable Qt item
pointer. That was a required correctness issue for the activation callbacks.

Inaccessible entries were correctly inert, but their tooltip-only diagnostic is
not runtime screen-reader evidence; that remains an explicit accessibility
limit rather than a claimed PASS.

## Parent correction reviewed by source evidence

The parent removed `itemActivated` entirely, added `_WorkspaceTree` with a
keyboard-only Enter/Return handler, emits only for a non-null current item,
and added a null guard before item data access. Existing mouse signals remain
the only mouse routes. The parent source probe, compile, Ruff, and format
checks passed after the correction.

## Post-correction independent review

Turing the 2nd / Luna max completed a read-only post-correction review with
PASS and no remaining required findings. The review confirmed that only
Enter/Return emits the keyboard signal, no live `itemActivated` connection
remains, mouse click/double-click routes are unchanged, nullable items are
rejected before data access, inaccessible kinds are inert, and the panel has
no application/service or filesystem dependency. No files were edited, no
worktree was used, and no Qt runtime or tests were run.

The official Qt contract used by the initial review is:
<https://doc.qt.io/qt-6/qtreewidget.html#itemActivated>. Native Qt event
ordering, focus traversal, screen-reader behavior, fonts, DPI, and runtime
interaction remain unrun; the PASS is a source review with those limits.
