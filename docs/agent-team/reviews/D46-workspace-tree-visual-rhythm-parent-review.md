# D46 parent review — workspace tree visual rhythm

- **Delivery:** D46 / ARCH-36 / UI-32
- **Date:** 2026-08-11
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

## Scope and architecture decision

D46 changes only the existing workspace tree's view interaction presentation
configuration and centralized QSS. `WorkspacePanel` now requests alternating
rows, full-row single selection, uniform heights, and middle elision. Theme QSS
adds explicit alternate and non-selected alternate-hover surfaces using
existing tokens. Signals,
semantic item routing, MainWindow policy, and application boundaries remain
unchanged.

The initial independent review found a real selector-specificity defect in the
first version. The parent corrected the alternate-hover selector to exclude
selected rows, reran the source probe and static checks, and requested a
bounded re-review. Locke the 2nd / Luna max returned no conclusion after two
further waits; no independent PASS is claimed.

The requested architecture consultation was attempted with Euclid the 2nd /
Luna max. Two bounded waits returned no conclusion and the agent was closed;
no architecture PASS is claimed. The independent review is Locke the 2nd /
Luna max and is recorded separately.

## Static review findings

- `QAbstractItemView` is used only for standard view-selection configuration;
  no domain/application type enters the widget.
- Existing click, double-click, and keyboard signal connections remain in the
  same panel code.
- `SelectRows` and `SingleSelection` make the visual highlight predictable
  without changing the semantic callback payload.
- `UniformRowHeights` and `ElideMiddle` make dense/long-name rows stable while
  preserving the underlying `Path` stored in item data.
- The alternate-hover selector excludes selected rows, so selected and
  selected-disabled rules cannot be visually overwritten by the alternate
  hover surface.

## Simplification assessment

The view flags plus two QSS selectors are the smallest complete view change.
No custom delegate, timer, animation, new token, or coordinator is needed.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, and motor-control requirements are
not applicable. Public CloudWeGo material is an engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.
Qt's public style-sheet syntax and `QAbstractItemView` references were used
for the selector-specificity correction and standard view configuration:
<https://doc.qt.io/qt-6/stylesheet-syntax.html> and
<https://doc.qt.io/qt-6/qabstractitemview.html>.

## Authorized non-destructive validation

- D46 visual-rhythm source probe — PASS.
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

Native QSS specificity, actual row metrics, font elision, keyboard selection,
screen-reader output, DPI, cross-machine rendering, and release-owner gates
remain unrun under the active no-launch policy.
