# D48 parent review — document-tab state clarity

## Scope and decision

- **Delivery:** D48 / ARCH-38 / UI-34
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

D48 adds only a presentation marker for unsaved tabs. `icons.py` owns the
authored `MODIFIED` glyph. `DocumentTabSurface` owns the icon projection and
theme refresh, while MainWindow remains the owner of dirty interpretation,
save/recovery/close policy, tab lifecycle, and document operations.

The architecture consultation was attempted with Beauvoir the 2nd / Luna max;
two bounded windows returned no conclusion. No architecture PASS is claimed.

## Source findings

- `DocumentTabSurface` stores a marker value aligned with its existing tab
  record list; add and remove update both lists at the same index.
- `set_modified()` updates only the presentation marker and does not inspect or
  mutate document state.
- `refresh_icons()` retints all current tabs from the current palette. New tabs
  call the same icon projection during `add_tab()`.
- MainWindow passes `state.dirty or editor.is_modified()` when adding and
  updating a tab, and retains the existing `*` title projection.
- Tab close/current callbacks, document event publication, save/recovery,
  startup, and close guards are untouched.
- The marker uses an accent-colored dot plus a document outline, while the
  existing title asterisk supplies a non-color cue.

## Simplification assessment

The surface API is the smallest complete change: `modified` at add time,
`set_modified()` for state transitions, and `refresh_icons()` for theme changes.
A custom delegate, widget-per-tab, or state callback would add coupling and
native rendering risk without improving the contract. No new signal, timer,
service, persistence value, or test-only asset was added.

## Authorized non-destructive validation

- D48 document-tab state source probe — **PASS**.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- `pwsh -NoProfile -File scripts\package.ps1` — **PASS**; root/dist portable
  candidates match at 38,417,875 bytes with the recorded SHA-256.
- `scripts\verify_handoff.ps1` — **PASS**.
- `scripts\check.ps1` — **PASS**.
- `scripts\verify_release_handoff.ps1` — **EXPECTED NO-GO**; 10 open gates and
  three known report-binding failures remain.
- Independent Luna review window — **NO_CONCLUSION** after two bounded waits;
  no independent PASS is claimed.
- No unit tests, mocks, fixtures, test-only assets, QApplication, Qt/EXE
  startup, screenshots, deployment, or hardware operation were created or
  run.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; MCU, BSP/HAL, RTOS, ISR/DMA, driver,
boot, Flash/NVM, power, motor-control, and vendor-manufacturer requirements
are not applicable. Public CloudWeGo sources are engineering references only;
no private ByteDance standard, certification, or compliance claim is made.

## Limits and disposition

Static evidence cannot prove native tab icon metrics, screen-reader output,
queued callback timing, runtime tab interaction, DPI, clean-machine,
cross-machine, or release-owner behavior. The bounded slice is accepted with
those limits and remains subject to the open runtime/release gates.
