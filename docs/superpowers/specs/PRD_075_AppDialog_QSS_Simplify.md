# PRD-075 AppDialog QSS Simplify Specs

## Goal

Move `AppDialog` visual styling from C++ inline `setStyleSheet()` strings into the three theme QSS files.

## Non-Goals

- Do not change `AppDialog` public APIs, promise behavior, modal behavior, layout, or animation timing.
- Do not introduce new dialog classes or replace existing `AppDialog` call sites.
- Do not modify CMake, launch scripts, resources, or build output paths.

## Constraints

- Follow `CLAUDE.md`, `01-project-overview.md`, `02-workflow.md`, `03-architecture.md`, `04-coding-standard.md`, `05-ui-standard.md`, and `07-directory-structure.md`.
- Keep existing object names: `appDialog`, `dialogIcon`, `dialogTitle`, `dialogMessage`, `dialogCancelBtn`, `dialogConfirmBtn`.
- Use a dynamic `dialogType` property with values: `confirm`, `warning`, `error`, `information`.
- Theme-specific colors must live in `resources/themes/*.qss`.

## Acceptance Criteria

- `src/core/widgets/AppDialogContent.cpp` no longer calls `setStyleSheet()`.
- All three theme files style `AppDialog` content and confirm button states.
- Confirm and information dialogs keep the accent confirm-button color; warning and error dialogs keep their distinct semantic colors.
- `doctor.ps1 -RunBuild -RunLaunch` succeeds, or the exact failure is recorded.
- `simplify-scan.ps1` report is refreshed.

## Validation Commands

```powershell
rg -n "setStyleSheet" .\src\core\widgets\AppDialogContent.cpp
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 -RunBuild -RunLaunch
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\simplify-scan.ps1 -MaxFiles 5 -OutFile .\docs\reviews\simplify\latest.md
```
