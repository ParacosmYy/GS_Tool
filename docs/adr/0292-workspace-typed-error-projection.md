# ADR-0292: Workspace typed-error projection

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D248 / ARCH-226

## Context

Workspace navigation failures were flattened in
`WorkspaceNavigationCoordinator.fail()` with `str(error)` before reaching
the workspace panel. The panel then applied `localize_message()` to that raw
English text. As a result, a Chinese user opening a folder could receive an
unclassified English `PermissionError` or `FileNotFoundError`, even though
the shared presentation i18n layer already knew how to localize those typed
exceptions for document dialogs.

## Decision

Extend the existing workspace error projection seam, without moving policy:

- `WorkspaceNavigationView.show_error()` and
  `WorkspaceSurface.show_error()` accept `str | Exception`;
- `WorkspacePanel.show_error()` calls `localize_exception()` for an
  exception and retains `localize_message()` for ordinary string feedback;
- the navigation coordinator forwards the original exception object.

Invalid-result messages remain strings and keep their existing localization.
Workspace service ownership, operation tracking, session-restore completion,
loading state, page retention, and notification policy remain unchanged.

## Boundaries and alternatives

- The error object is retained only across the synchronous presentation call;
  it is not stored, serialized, or re-raised.
- The workspace panel reuses the existing i18n boundary rather than adding a
  workspace-specific translation table.
- English remains compatible because `localize_exception()` returns
  `str(error)` for `en-US`.
- No EXE/Qt launch, registry, installer, updater, or unit-test asset is used.

## Public-source applicability and review

Python 3.12 first-party documentation for
[built-in exceptions](https://docs.python.org/3.12/library/exceptions.html)
is the applicable engineering reference for preserving exception identity and
codec/path metadata. No Qt API or manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or manufacturer
requirement is claimed. Embedded C/C++, MCU, RTOS, and related public-source
workflows are not applicable.

The architecture role `Dewey the 7th / Luna max` returned `NO_CONCLUSION`
after a bounded wait and closure. The independent reviewer `Beauvoir the 7th /
Luna max` likewise returned `NO_CONCLUSION` after a bounded wait and closure.
No child approval is claimed. Parent review is `PASS`; behavior-preserving
simplification is `PASS`.

## Evidence and limits

- `D248-WORKSPACE-TYPED-ERROR-ROUTING=PASS`,
  `D248-WORKSPACE-CHINESE-ERROR=PASS`, and
  `D248-WORKSPACE-ENGLISH-COMPATIBILITY=PASS`.
- `D248-COMPILEALL=PASS`, `D248-RUFF=PASS`, and `D248-FORMAT=PASS`.
- PS5.1 and PS7 package builds passed; final PS7/root artifact identity is
  SHA-256 `E016DF4FE44370527CA916C531F8E595FDB796D8792ABE468925FF0139D2D57B`,
  38,576,424 bytes, source revision
  `tree-sha256:368041dba7a50b9cef404c4036fd1337b76d253c370d099ec3f7c25cf295be11`.
- The frozen archive contains the `__main__` outer entry,
  `quillforge.app`, `quillforge.composition`, workspace navigation modules,
  PyQt6, and `qwindows.dll`; the warning file has 25 non-empty lines.

Native EXE/Qt startup, native folder/file dialogs, shell association launch,
clean-machine behavior, signing, installer, updater, support, and release-owner
evidence remain open under the active no-launch/non-destructive policy.
