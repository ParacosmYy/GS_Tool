# D66 parent review — dialog action hierarchy

| Field | Value |
|---|---|
| Delivery | `D66 / UI-40` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The change adds presentation-only object names to the plugin catalog, plugin
status, workspace search, and settings dialogs. The centralized theme adds a
quiet button role and a shared dialog action rail. Existing primary and
warning action selectors are reused.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. Existing button connections,
  enablement predicates, button order, locale updates, and dialog close paths
  are unchanged.
- **Readability/simplicity:** PASS. Four local object-name assignments and
  one shared QSS role replace no behavior and avoid a new widget hierarchy.
- **Architecture:** PASS with limits. Dialogs remain presentation projections;
  theme remains the single visual-token owner; no service or MainWindow policy
  moves.
- **Security:** PASS by scope. No input, process, persistence, plugin trust,
  or external integration path changed.
- **Performance:** PASS by scope. QSS selector matching and static metadata
  add no unbounded work or new dependency.

## Simplification assessment

No further safe simplification was identified. Reusing `primaryAction` and
`warningAction` is simpler than introducing aliases or a dialog base class;
`quietAction` is the one genuinely missing semantic role. The action rail is
shared only through QSS and an existing Qt control, so no speculative
component abstraction is warranted.

## Authorized non-destructive validation

- `D66-dialog-action-hierarchy-probe=PASS`.
- `D66-dialog-contrast-probe=PASS`; default/hover/pressed state combinations
  remain at least 4.5:1 for Ink/Violet, Paper/Sand, and Sakura/Pop.
- Compileall, Ruff, and format pass before documentation/package sync.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native rendering, keyboard traversal, accessibility tooling, DPI/font metrics,
runtime startup, clean-machine, cross-machine, and release-owner evidence
remain open. Embedded C/C++ and vendor-public-source requirements are N/A.
