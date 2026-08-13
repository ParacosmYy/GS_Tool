# D186 parent review — workspace file-entry closure

Date: 2026-08-11

## Conclusion

`PASS` for the bounded source change, with native dialog/runtime limits kept
explicit.

## Review notes

- `WorkspacePanel` owns the new button and semantic signal only.
- `WorkspaceSurfaceCallbacks` is the single forwarding contract; MainWindow
  binds the callback to its existing `_open_document` policy entry point.
- The existing FileDialogSurface, picker admission, open admission, worker
  dispatch, and DocumentService boundaries are reused; no direct service call
  or duplicate path was added.
- The file action follows existing loading enablement and uses the existing
  localized icon/theme path.
- Static probes confirm English/Simplified Chinese labels and the complete
  source call chain.

## Simplification assessment

`PASS`: one semantic signal plus one callback is the smallest cohesive change;
adding a second picker/coordinator would duplicate existing policy.

## Limits

Native file-dialog selection, GUI startup, screenshots, accessibility, DPI,
clean-machine, cross-machine, and release evidence were not run.
