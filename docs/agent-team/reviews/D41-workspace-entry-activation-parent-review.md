# D41 parent review — workspace-entry activation boundary

- **Delivery:** D41 / ARCH-31 / UI-27
- **Date:** 2026-08-11
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits for the bounded static/source slice

## Scope and architecture decision

D41 keeps the existing workspace mouse contract and adds the missing keyboard
activation route. A presentation-only `_WorkspaceTree` intercepts Enter/Return
and emits a keyboard-only signal to one `_emit_item_intent` helper. The helper
maps only `file` to `file_requested` and `directory` to
`directory_requested`; inaccessible and metadata-only rows are ignored.

The existing `itemClicked` file route and `itemDoubleClicked` directory route
remain intact for compatibility. MainWindow continues to own workspace
containment, startup/busy guards, TaskRunner submission, document opening,
directory loading, stale generations, notifications, and error policy. No
application or infrastructure dependency was added to the panel.

The required Terra architecture consultation was attempted with Dewey the 2nd
/ Terra max. Two bounded waits returned no conclusion and the agent was
closed; no architecture PASS is claimed. Faraday the 2nd / Luna max completed
an independent review and correctly returned FAIL for two issues: the
style-dependent `itemActivated` route could duplicate mouse delivery, and
nullable Qt item pointers were dereferenced without a guard. The parent
removed that signal path, added a keyboard-specific tree subclass, and added
null safety. Turing the 2nd / Luna max then completed the post-correction
independent review with PASS and the explicit limits recorded in the
independent-review record.

## Static review findings

- `_WorkspaceTree.keyPressEvent` emits keyboard activation only for
  Enter/Return when a current item is non-null.
- Mouse click and double-click handlers now delegate to the same helper with
  an explicit expected kind, preserving their previous behavior.
- Keyboard activation delegates both supported entry kinds to the matching
  semantic callback.
- `_item_path_and_kind` treats a nullable Qt item as a no-op before reading
  item data.
- The helper does not touch filesystem, document, TaskRunner, containment, or
  notification services.
- Inaccessible entries carry no file/directory kind and therefore emit no
  activation callback.
- The style-dependent `itemActivated` signal is not used, so mouse routing is
  not duplicated by the keyboard route.

## Simplification assessment

The smallest corrected change is one tiny tree key handler plus one
presentation helper. It removes duplicated path/kind dispatch without using a
style-dependent Qt signal or introducing a coordinator, service, event bus, or
application contract. No further safe simplification is required for D41.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material remains an engineering reference
only; no private ByteDance standard, certification, or compliance claim is
made. Applicability is recorded in ADR-0066.

## Authorized non-destructive validation

- D41 workspace-entry activation source probe — PASS: Enter/Return-only
  keyboard route, null safety, matching callbacks, unchanged mouse routes,
  and no service imports are present.
- `uv run python -m compileall -q src/quillforge/presentation/workspace_panel.py`
  — PASS.
- `uv run ruff check src/quillforge/presentation/workspace_panel.py` — PASS.
- `uv run ruff format --check src/quillforge/presentation/workspace_panel.py`
  — PASS.
- No unit tests, mocks, fixtures, harnesses, test-only assets, QApplication,
  screenshots, deployment, or hardware operation were created or run.

## Limits

Native Qt signal ordering, real mouse/keyboard activation, focus traversal,
accessibility, fonts, DPI, cross-machine rendering, and release-owner gates
remain unrun under the active no-launch policy. D41 corrects the activation
route; it is not runtime visual acceptance or a complete document coordinator
rewrite.
