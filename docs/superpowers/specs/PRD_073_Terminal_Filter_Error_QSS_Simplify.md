# PRD-073 - Terminal Filter Error QSS Simplify

## 1. Goal

Remove the inline `setStyleSheet()` error border from `TerminalFilterBar` and use the existing QSS `hasError` dynamic property instead.

## 2. Non-goals

- Do not redesign the terminal filter UI.
- Do not change filtering behavior, regex validation, history behavior, or emitted signals.
- Do not edit `MainWindow`, `PanelManager`, or build scripts.
- Do not add new classes.
- Do not create a second build directory.

## 3. Required Constraints

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/05-ui-standard.md`

## 4. Change Scope

| Scope | Path | Allowed action |
|------|------|----------------|
| Implementation | `src/terminal/filter/TerminalFilterBar.cpp` | modify |
| Header | `src/terminal/filter/TerminalFilterBar.h` | modify only if a helper declaration is needed |
| QSS themes | `resources/themes/*.qss` | read-only unless existing `hasError` is missing |
| Forbidden | `CMakeLists.txt`, `EmbedDebug.bat`, `src/core/mainwindow/`, `src/core/panels/` | no edits |

## 5. Acceptance

- [ ] `TerminalFilterBar.cpp` no longer calls `setStyleSheet`.
- [ ] Invalid regex still sets a tooltip and visible error state through `hasError=true`.
- [ ] Clearing the pattern clears the tooltip and resets `hasError=false`.
- [ ] Existing theme QSS handles `QLineEdit[hasError="true"]`.
- [ ] Build passes.
- [ ] `EmbedDebug.bat` launches successfully after the change.

## 6. Failure Conditions

- Inline style or hardcoded color remains in `TerminalFilterBar`.
- Regex validation behavior changes.
- A new theme-specific selector is added unnecessarily.
- Build or launch validation fails.

## 7. GO Config

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "cmake --build .\\build --config Release --parallel 4"
  ],
  "check": [
    "rg -n \"setStyleSheet\" src\\terminal\\filter\\TerminalFilterBar.cpp",
    ".\\EmbedDebug.bat"
  ],
  "fix": []
}
```

## 8. BATCH Decision

- Needs BATCH: no.
- Reason: one small file-level technical debt cleanup.
- Parallelism: 1.

## 9. LOOP Route

- Doctor: build and launch validation.
- Debug: if regex behavior regresses, capture with `tools/debug-trace.ps1`.
- Simplify: this task removes one inline styling responsibility from C++ and returns styling to QSS.
