# D45 parent review — workspace file activation consistency

- **Delivery:** D45 / ARCH-35 / UI-31
- **Date:** 2026-08-11
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

## Scope and architecture decision

D45 keeps the existing WorkspacePanel → WorkspaceSurface → MainWindow
semantic route. WorkspacePanel now activates both file and directory kinds on
double-click while preserving single-click file opening and Enter/Return. The
MainWindow callback retains startup, busy, containment, tab identity, async
dispatch, notification, and DocumentService policy.

The requested architecture consultation was attempted with Avicenna the 2nd /
Luna max. Three bounded waits returned no conclusion and the agent was closed;
no architecture PASS is claimed. The independent code-review window was
Feynman the 2nd / Luna max; two bounded waits returned no conclusion and the
agent was closed, as recorded in the companion review file.

## Static review findings

- `_on_item_double_clicked` calls the existing semantic item-intent helper,
  so file and directory kinds still map to their matching signals.
- `_on_item_clicked` continues to restrict single-click activation to files;
  inaccessible and marker entries remain inert.
- `_open_workspace_file` rejects non-Path input, startup restore, and busy
  state before applying the existing workspace containment check.
- An existing path-backed tab is selected through DocumentTabSurface; a new
  path still uses `_start_open` and the existing TaskRunner/DocumentService
  boundary.
- The localized workspace status hint describes both supported file gestures
  in English and Simplified Chinese.

## Simplification assessment

The two-line route adjustment and local guards are the smallest complete fix.
No timer, new signal, new coordinator, or application-layer change is needed.
The current generic busy guard plus existing-tab lookup prevents a fast
double-click from creating duplicate work without another state owner.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material is an engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.

## Authorized non-destructive validation

- D45 activation source probe — PASS.
- `uv run python -m compileall -q src` — PASS.
- `uv run ruff check src` — PASS.
- `uv run ruff format --check src` — PASS.
- `pwsh -NoProfile -File scripts\package.ps1` — PASS; root/dist portable
  candidates match at 38,415,189 bytes with the recorded SHA-256.
- `scripts\verify_release_handoff.ps1` — expected NO-GO; 10 open gates and
  three known mechanical report-binding failures remain.
- No unit tests, mocks, fixtures, test-only assets, QApplication, screenshots,
  deployment, or hardware operation were created or run.

## Limits

Native Qt mouse-event ordering, double-click timing, queued worker delivery,
runtime file opening, focus/accessibility behavior, DPI/font metrics,
cross-machine rendering, and release-owner gates remain unrun under the active
no-launch policy.
