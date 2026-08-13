# D53 parent review — Find Match snapshot boundary

## Scope and decision

- **Delivery:** D53 / ARCH-43
- **Reviewer:** Architect (parent integration review)
- **Decision:** accepted-with-limits; independent review status recorded separately

`FindMatchTracker` replaces MainWindow's anonymous match tuple with an
immutable, typed snapshot and explicit clear/match operations. MainWindow
still calls the editor find adapter, reads live selection and selected text,
performs single replacement, owns busy/feedback policy, and invalidates on
the existing criteria/document/tab lifecycle routes.

The required architecture consultation was attempted with Singer the 2nd /
Luna max. Two bounded windows returned no conclusion; no architecture PASS is
claimed. The independent review was attempted with Sagan the 2nd / Luna max.
Two bounded windows returned no conclusion and the agent was closed; no
independent PASS is claimed.

## Source findings

- `record()` stores a snapshot only when a find succeeded with a selection;
  otherwise it clears the previous match.
- `matches()` compares tab identity, query, case sensitivity, selection bounds,
  and content version, and rejects a live missing selection.
- MainWindow keeps the existing selected-text equality check, so a matching
  coordinate alone cannot replace different text.
- Existing `_invalidate_find_match` call sites now clear the tracker without
  changing criteria-changed, editor-modified, content-version, tab-change, or
  close behavior.
- The tracker has no Qt or service dependencies and owns no editor/UI policy.

## Simplification assessment

The tracker is the smallest complete extraction: one immutable value object,
one optional slot, one record operation, one exact-match predicate, and one
clear operation. It avoids a generic search coordinator, editor dependency,
signals, or duplicated invalidation callbacks. Keeping the selected-text
comparison in MainWindow preserves the existing policy and avoids moving
editor semantics into a state holder. No further simplification was justified.

## Authorized non-destructive validation

- `D53-find-match-identity-and-stale-probe=PASS`.
- `D53-find-match-qt-free-and-policy-boundary-probe=PASS`.
- `uv run python -m compileall -q src` — **PASS**.
- `uv run ruff check src` — **PASS**.
- `uv run ruff format --check src` — **PASS**.
- Package identity and release verifier evidence are recorded after the D53
  package rebuild.
- Independent Luna review window — **NO_CONCLUSION** after two bounded waits;
  no independent PASS is claimed.
- No unit tests, mocks, fixtures, test-only assets, QApplication, Qt/EXE
  startup, screenshots, deployment, or hardware operation were created or
  run.

## Public-source applicability and embedded gate

This is Python/PyQt6 desktop code; embedded C/C++, MCU, BSP/HAL, RTOS,
ISR/DMA, driver, boot, Flash/NVM, power, motor-control, and vendor
requirements are not applicable. Public CloudWeGo sources remain engineering
references only; no private ByteDance standard, certification, or compliance
claim is made.

## Limits and disposition

Static evidence cannot prove native selection timing, editor focus behavior,
or runtime replacement interaction. The bounded match state is accepted with
those limits and remains subject to the open runtime/release gates.
