# D9 / UI-11 state-clarity parent review

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | `src/quillforge/presentation/theme.py` and the UI-11 ledger records |
| Owner | Architect (parent) |
| Route | Luna/max/Fast delegated role reviews; parent integration |
| Result | `accepted-with-limits` after parent correction |

## User outcome

The shared visual system now closes the remaining state-visibility gaps that made
the shell look flat: focused item views have an accent boundary, alternating list
rows have a stable secondary surface, selected-disabled entries are muted, the
primary workspace action cannot look actionable while disabled, and disabled
checkbox indicators remain visibly distinct.

## Architecture decision

Keep the change entirely inside the presentation stylesheet. `apply_theme()` is
the composition-root projection boundary, and `MainWindow` reapplies it after a
successful settings save. No domain/application/infrastructure module, Qt
signal, editor adapter contract, workspace behavior, or task lifecycle changed.
The existing `ThemeColors` tokens are reused; no second style system or new
widget-local stylesheet was introduced.

## Fixed-role inputs

- Project Manager / Feynman: UI-11 was not previously indexed; define a bounded
  UI slice, record acceptance IDs, and preserve no-launch limits.
- Product / Erdos: define the user result as distinguishable focus, selection,
  alternate, disabled, primary, and checkbox states across Sakura Pop and legacy
  themes; keep business behavior out of scope.
- Developer 1 / Heisenberg: `theme.py` is the correct high-cohesion boundary;
  preserve the editor adapter and application/service seams.
- Developer 2 / Archimedes: identified the concrete `primaryAction:disabled`
  specificity gap in `WorkspacePanel` loading state.
- QA / Planck: authorized compileall/Ruff/format/JSON/handoff checks; Qt/EXE
  startup, screenshots, and interactive visual acceptance remain prohibited.

## Independent review and parent action

The first independent Luna review (Raman) returned `REVISE` with two concrete
findings:

1. `QAbstractItemView::item:selected` appeared before `::item:alternate`, so
   equal-specificity declarations could let the alternate-row surface obscure a
   selected row.
2. `QPushButton#primaryAction:disabled:focus` duplicated the declarations of
   the disabled rule and was unnecessary after the focus rule was explicitly
   gated with `:!disabled`.

The parent moved `::item:alternate` before `::item:selected` and removed the
duplicate disabled-focus selector. The remaining review observations were
accepted: selected-disabled rules are placed after the workspace/list inactive
rule; combo-popup selected-disabled is after its selected rule; and disabled
checkbox rules are after checked/hover rules. A time-boxed follow-up Luna
review window (Bohr) produced no conclusion before shutdown; therefore this
handoff does not claim a fresh child PASS. Parent verification below is the
authoritative integration evidence.

Qt's official style-sheet documentation describes chained pseudo-states,
selector specificity, and conflict resolution:

- https://doc.qt.io/qt-6/stylesheet-syntax.html
- https://doc.qt.io/qt-6/stylesheet-reference.html

## Simplification assessment

Safe simplification was applied only where behavior was provably unchanged:
the redundant `primaryAction:disabled:focus` selector was removed. The
workspace/list-specific selected-disabled rule, the combo-popup rule, and the
checkbox indicator rules remain separate because their selector specificity or
subcontrol boundary differs. No broader refactor was judged safe without Qt
runtime rendering evidence.

## Evidence and limits

- `theme.py:280-303`: item-view border, focus, alternate, selected, and
  selected-disabled ordering.
- `theme.py:560-580`: workspace/list selected, inactive, and terminal disabled
  ordering.
- `theme.py:616-646`: primary action normal, hover, pressed, focus, and disabled
  states.
- `theme.py:678-690`: combo popup selected and selected-disabled states.
- `theme.py:700-729`: checkbox normal, checked, disabled, and checked-disabled
  indicator states.
- `uv run python -m compileall -q src/quillforge` — PASS.
- `uv run ruff check src/quillforge` — PASS.
- `uv run ruff format --check src/quillforge` — PASS.
- Acceptance and delivery-register JSON parse — PASS.
- `scripts/verify_handoff.ps1` — PASS.
- `scripts/check.ps1` — PASS.
- `scripts/package.ps1` — PASS; root/dist `QuillForge.exe` are both
  `38,365,225` bytes with SHA-256
  `8AF11EE8D661FDFF2A564DE6F48D7056D3A41887A53B969AA5CAF99AE5B115E6`.
- `scripts/verify_release_handoff.ps1` — expected NO-GO; the current dossier
  binds to the new artifact and retains exactly three stale-runtime mechanical
  failures plus ten open release gates.

Not run: Qt/QApplication/EXE startup, screenshots, actual hover/focus/click
rendering, formal contrast calculation, screen-reader, font/DPI/native-style,
clean-machine, and cross-machine checks. The project no-launch boundary makes
those user-owned follow-up evidence, not a source-only completion claim.

## Parent disposition

`accepted-with-limits`: the presentation-only correction is integrated and
statically verified. Runtime visual acceptance and release-level evidence remain
open under the project policy.
