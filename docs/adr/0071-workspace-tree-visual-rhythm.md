# ADR-0071: workspace tree visual rhythm

- **Status:** accepted-with-limits; D46 / ARCH-36 / UI-32 bounded slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The workspace panel had explicit selected, hover, focus, and disabled QSS,
but the tree view still used platform-default row selection geometry. Long
names and dense rows made the panel feel visually flat and made the selected
entry harder to scan.

## Decision

Keep the existing `WorkspacePanel` and centralized theme boundary. Configure
the tree's view interaction presentation for alternating rows, full-row single
selection, stable row heights, and middle ellipsis for long entry names. Add
explicit alternate and non-selected alternate-hover backgrounds using
existing theme tokens. Preserve all existing semantic signals and item-kind
routing.

No application/service/filesystem policy, new widget, new token, or animation
owner is introduced; `SelectRows` and `SingleSelection` remain standard view
selection configuration rather than a new domain state owner.

## Invariants

1. `itemClicked`, `itemDoubleClicked`, and keyboard activation connections are
   unchanged.
2. File/directory/inaccessible intent routing remains owned by the existing
   presentation helper and MainWindow callbacks.
3. Selected rows remain visibly stronger than alternate or non-selected hover
   rows, including when an alternating row is hovered.
4. The tree permits only one selected entry and highlights the complete row.
5. Theme changes continue to use the existing tokenized stylesheet.

## Alternatives considered

- **Leave platform row metrics unchanged:** rejected because they caused the
  reported lack of visual distinction and vary across native styles.
- **Introduce a custom delegate or icon/widget row:** rejected as a larger
  rendering surface with higher DPI/accessibility risk.
- **Add a new visual token palette:** rejected because the existing theme
  tokens already express the needed surface hierarchy.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code, not embedded C/C++ or firmware;
MCU/vendor requirements are not applicable. Public CloudWeGo material remains
transferable engineering reference only and does not establish a private
ByteDance standard, certification, or compliance claim:

- <https://www.cloudwego.io/about/>
- <https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/>
- <https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/>

The applicable framework references for this QSS/view slice are Qt's public
documentation on [style-sheet pseudo-state negation and specificity](https://doc.qt.io/qt-6/stylesheet-syntax.html)
and [QAbstractItemView selection configuration](https://doc.qt.io/qt-6/qabstractitemview.html).

## Verification target

- A source probe proves all view configuration, selector coverage, and signal
  preservation.
- Compile, Ruff, format, JSON, handoff, package, and release no-go evidence
  are recorded.
- Independent review records a conclusion or explicit no-conclusion state.
- No unit tests, Qt startup, screenshots, deployment, or hardware operation
  are created or run under the active policy.

## Limits and simplification

This is the smallest complete view-presentation change: native view settings
plus two selectors. A custom delegate, new theme tokens, and animation were
not added because they would increase rendering and lifecycle risk without
addressing the immediate row hierarchy gap. Native QSS rendering, font/DPI,
accessibility, cross-machine appearance, and release-owner gates remain
unrun.
