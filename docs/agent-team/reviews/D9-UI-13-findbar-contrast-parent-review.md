# D9 / UI-13 parent review: FindBar hierarchy and accent contrast

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | FindBar action hierarchy and centralized bright-accent foreground contrast |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

The FindBar now gives the current mode one clear primary action: Next while
finding and Replace while replacing. Replace All has an explicit warning role.
The bright Sakura/ink/paper accent endpoints now select a foreground that is
readable across the complete static endpoint set, including the reported amber
(`砂金`) path.

## Architecture decision

- Keep the change in `presentation`; no domain, application, infrastructure, or
  editor-engine ownership moved.
- Reuse the existing semantic QSS contract. `FindBar.show_find()` projects the
  current mode through `objectName` and repolishes only the two role-bearing
  buttons so the existing stylesheet resolves the role immediately.
- Keep Replace All on a separate `warningAction` role instead of making a
  destructive-looking action share the primary gradient.
- Compute `ThemeColors.on_accent` once from the centralized theme token set,
  selecting the better of white and the dark ink foreground for all accent,
  pink, and gold endpoints. The helper is pure and does not depend on Qt.
- Preserve FindBar signals, Enter/Shift+Enter/Esc routing, operation locking,
  locale refresh, and the existing editor/application boundary.

## Fixed-role input and ownership

| Role | Agent | Contribution | Disposition |
|---|---|---|---|
| Architect | parent | Integrated the urgent contrast fix and UI-13 slice, reviewed the boundary, and owns verification/handoff | accepted |
| Project Manager | Hume / Luna | Recommended a bounded FindBar action hierarchy and static/package evidence | incorporated |
| Product | James / Luna | Recommended one current-mode primary action, a distinct Replace All warning, and behavior preservation | incorporated |
| Developer 1 | Averroes / Luna | Confirmed the change is presentation-only and must not cross TaskRunner/status/application seams | incorporated |
| Developer 2 | Kant / Luna | Did not return a bounded conclusion before shutdown | no conclusion; not used as approval |
| QA | Huygens / Luna | Supplied compile/Ruff/format/JSON/handoff/check/package evidence and no-launch limits | incorporated |

The fixed design reviewer Meitner / Luna did not return a conclusion in the
bounded window and was closed. The independent code reviewer Noether / Luna
also did not return a conclusion in the bounded window and was closed. No child
PASS is claimed for either review; the Architect remains responsible for the
final source and evidence review.

## Root cause and simplification assessment

The earlier centralized palette used white `on_accent` for every theme and
accent endpoint. Static WCAG-style luminance calculation showed that this was
not sufficient for bright Sakura/ink endpoints and that some amber warning
states were also below the intended 4.5:1 text ratio. The fix makes the
foreground selection data-driven and audits all supported theme/accent
endpoints instead of patching only the reported color.

The smallest complete implementation is one short FindBar role helper, one
pure contrast-selection helper in the existing theme module, corrected paper
accent tokens, and one centralized `warningAction` selector. No widget-local
stylesheet, duplicate color engine, signal wrapper, or application-layer
state was introduced. No further behavior-preserving simplification was
identified.

## Public source applicability

This change touches Python/PyQt6 presentation code only. No embedded C/C++,
MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, bootloader, Flash/NVM, power, or
motor-control code was modified. The embedded vendor-source workflow and
manufacturer-requirement applicability are therefore `N/A`; no MISRA,
ISO 26262, automotive, or certification claim is made.

## Independent review result

Independent child reviews: `NO CONCLUSION` within the bounded wait for Meitner
(design) and Noether (code). The parent review is `accepted-with-limits` based
on the bounded role inputs, source inspection, pure contrast probe, behavior
boundary probe, and repository/package gates below. Runtime Qt rendering is
explicitly not represented as reviewed.

## Authorized verification

- `uv run python -m compileall -q src\\quillforge\\presentation` — PASS.
- `uv run ruff check src\\quillforge\\presentation\\find_bar.py src\\quillforge\\presentation\\theme.py` — PASS.
- `uv run ruff format --check src\\quillforge\\presentation\\find_bar.py src\\quillforge\\presentation\\theme.py` — PASS after formatting.
- Static contrast probe — PASS: 36 accent/pink/gold endpoint checks across
  three themes and four accent selections, plus warning normal-state text.
- UI-13 behavior-boundary probe — PASS: role projection, repolish, existing
  signal connections, and centralized selectors are present; no signal path
  was replaced.
- `scripts/verify_handoff.ps1` — PASS after final ledger synchronization.
- `scripts/check.ps1` — PASS after final ledger synchronization.
- `scripts/package.ps1` through PowerShell Core — PASS; root/dist SHA-256 is
  `18413C8D0136B434660D188D2E6A03DD76ECA4CAC1FC7B533813F62048CD267C`,
  `38367313` bytes, with source snapshot
  `tree-sha256:154ce7dc3f95d622e5402399f0bd73be03c92e549ef4e94aefd1edff9a5f64fe`.
- `scripts/verify_release_handoff.ps1` — expected `NO-GO`; the known three
  artifact/report consistency failures and ten external gates remain open.

## Unrun checks and reason

- QApplication/Qt startup, FindBar interaction, screenshots, and runtime
  visual acceptance — prohibited by the active project no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under the project R&D constraints.
- Clean-machine, native-style/DPI/font, screen-reader, cross-machine,
  signing, installer, update, deployment, and hardware checks — outside this
  local non-destructive source/package scope and not authorized.

## Known risks and limits

- QSS specificity and native style rendering still require authorized runtime
  review; the static contrast result is not a substitute for visual acceptance.
- `on_accent` is selected for all three gradient endpoints, but platform font
  rasterization, font availability, and DPI can still affect perceived UI.
- Release status remains `NO-GO`; UI-13 does not close D7/D8 or external release
  gates.

## Acceptance and evidence IDs

- Acceptance: `D9-AC11`, `S38`.
- Evidence: `src/quillforge/presentation/find_bar.py`,
  `src/quillforge/presentation/theme.py`, this review,
  `docs/handoffs/2026-08-10-d9-ui-13-findbar-contrast/handoff.md`,
  `tasks/plan.md`, `tasks/todo.md`, `scripts/verify_handoff.ps1`,
  `scripts/check.ps1`, `scripts/package.ps1`, and `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: Architect.
- Action: maintain the open D7/D8/release gates and begin the separately
  recorded enterprise-architecture baseline before the next cross-module
  migration slice.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `18413C8D0136B434660D188D2E6A03DD76ECA4CAC1FC7B533813F62048CD267C` /
  `38367313` bytes for both root and dist copies; source
  `tree-sha256:154ce7dc3f95d622e5402399f0bd73be03c92e549ef4e94aefd1edff9a5f64fe`.
- Packaging note: portable package rebuilt through PowerShell Core; this is
  artifact evidence, not release approval.

## Disposition

`accepted-with-limits`: the urgent bright-accent contrast defect and bounded
FindBar hierarchy slice are source-reviewed, statically verified, packaged,
and traceable. Runtime visual evidence and external release conditions remain
open.
