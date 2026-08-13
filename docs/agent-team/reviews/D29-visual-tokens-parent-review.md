# D29 parent review — visual endpoint contrast and shell rhythm

| Field | Value |
|---|---|
| Slice | D29 / UI-15 visual endpoint contrast and shell rhythm |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Newton the 2nd / Luna max; two bounded read-only wait windows returned no conclusion; no PASS is claimed |
| Scope | `theme.py`, visual-token/QSS contracts, ADR/spec/roadmap/acceptance/handoff projections |
| Decision | accepted-with-limits; parent static review plus explicit independent no-conclusion record |

## Outcome

The centralized visual system now derives `on_accent_gold` independently and
uses it for the amber/砂金 warning-action hover state. The command rail and
document-tab surface received bounded spacing, border, radius, and selector
hierarchy refinements from the same generated QSS source. Theme settings,
accent IDs, action roles, localization, keyboard behavior, and editor syntax
semantics remain unchanged.

## Parent review findings

### Contrast and behavior

- PASS by static reasoning: `_theme_colors()` derives `on_accent_gold` from
  the actual gold endpoint instead of reusing a foreground chosen for another
  gradient endpoint.
- PASS by authorized contrast probe: every supported theme/accent combination
  passed `>= 4.5:1` for accent, pink, gold, and status-attention pairs.
- PASS by source reasoning: `warningAction` keeps its warning object name,
  callbacks, pressed/focus/disabled states, and semantic role; only the filled
  hover foreground token changed.
- NOT RUNTIME-VERIFIED: Qt stylesheet precedence, native rendering, installed
  fonts, DPI, screenshots, screen-reader output, and cross-machine appearance
  remain unrun under the no-launch policy.

### Architecture and visual hierarchy

- PASS by source reasoning: `ThemeColors` and `_stylesheet()` remain the sole
  visual-token/QSS source; no widget-local stylesheet or second theme engine was
  introduced.
- PASS by source reasoning: `QToolBar#commandBar` and
  `QTabWidget#documentTabs` selectors are scoped to existing object names and
  preserve existing interaction states.
- PASS by source reasoning: no application service, document state, persistence
  adapter, or operation policy entered the theme module.

## Simplification assessment

The change removes a hidden cross-endpoint foreground assumption by adding one
semantic token and keeps the existing contrast helper as the only derivation
mechanism. It avoids duplicated per-theme branches and a runtime repair pass;
no further safe behavior-preserving simplification is required for D29.

## Independent review

Newton the 2nd / Luna max was assigned a bounded read-only visual/source review
with no write access, no Qt launch, and no test creation/run. Two bounded wait
windows returned no conclusion and the reviewer was closed. No child PASS or
FAIL is claimed; D29 is accepted only with parent static reasoning and the
explicit no-conclusion limitation.

## Public-source applicability and embedded gate

This slice is Python/PyQt6 desktop code, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are **N/A**
for MCU/vendor constraints because no firmware target, SDK, RTOS, ISR/DMA,
driver, protocol, boot, Flash/NVM, power, or hardware was changed. Public
architecture references are engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or a release-readiness claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS; 79 source files formatted |
| D29 contrast endpoint probe | PASS; all theme/accent accent, pink, gold, and status-attention pairs >= 4.5:1 |
| D29 QSS token-boundary probe | PASS; `on_accent_gold`, `warningAction`, `commandBar`, and `documentTabs` contracts present |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS; historical D29 root/dist SHA `E846F86BFE01551D7A8A0D9AA51461E40498DC5BA8DE4933E64C42C7E718FBC7`, size `38,394,480` bytes; source `tree-sha256:5f0656aaa24c33bf316da8f778b2cc8db53097af21ba23b18151fdb2669c9c46` |
| `scripts\verify_handoff.ps1` | PASS; D29 handoff/index/register/acceptance status and required sections are synchronized |
| `scripts\check.ps1` | PASS; D29 source, formatting, inventory, and static project checks passed |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; exit 1 with exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review returned no conclusion; no child PASS is represented.
- Runtime QSS/native style behavior, visual hierarchy, fonts, DPI,
  accessibility, and cross-machine rendering remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D29.

**Disposition:** `accepted-with-limits`; D29 visual token and shell-rhythm
refinement is statically verified by the parent with explicit independent
no-conclusion evidence and all runtime/external release gates open.
