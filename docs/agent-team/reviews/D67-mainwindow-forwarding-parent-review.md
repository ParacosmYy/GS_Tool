# D67 parent review — MainWindow file-dialog/About forwarding simplification

| Field | Value |
|---|---|
| Delivery | `D67 / ARCH-51` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The change deletes two MainWindow-only forwarding methods and routes existing
call sites to `FileDialogSurface.choose_save_path` and
`MessageSurface.show_about` directly. No surface implementation changed.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. Save As and dirty-close retain
  the same arguments and `_start_save` callback path; the About command keeps
  the same ID/title/menu and callable behavior.
- **Readability/simplicity:** PASS. The coordinator no longer advertises two
  false policy owners; direct calls make the existing surface boundary visible.
- **Architecture:** PASS with limits. File-dialog and message composition stay
  in presentation surfaces; MainWindow retains document and command policy.
- **Security:** PASS by scope. No filesystem write, user input validation,
  process, plugin, or persistence boundary changed.
- **Performance:** PASS by scope. One Python call frame is removed; no new
  work, dependency, or allocation path is introduced.

## Simplification assessment

No further safe simplification was identified in this slice. The direct calls
are clearer than preserving aliases, while existing policy-bearing methods
such as `_save_as_document` and `_close_tab` remain intact.

## Authorized non-destructive validation

- `D67-forwarding-simplification-probe=PASS`.
- Targeted compileall, Ruff, and format checks — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native file-dialog/About interaction, callback timing, accessibility, runtime
startup, clean-machine, cross-machine, and release-owner evidence remain open.
Embedded C/C++ and vendor-public-source requirements are N/A.
