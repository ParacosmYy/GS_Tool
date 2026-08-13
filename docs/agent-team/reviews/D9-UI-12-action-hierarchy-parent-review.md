# D9 / UI-12 parent review: dialog primary-action hierarchy

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | Settings Save and workspace-search Search action presentation |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

The Settings dialog's Save action and the workspace-search dialog's Search action now use the existing centralized primary-action presentation. Cancel and Close remain secondary controls, and the dialog/application signal paths are unchanged.

This is a bounded UI-12 slice responding to the user's report that controls lack enough visual highlighting and hierarchy. It improves scanability without introducing another theme engine or moving behavior ownership into the presentation layer.

## Architecture decision

- Reuse `src/quillforge/presentation/theme.py`'s existing `QPushButton#primaryAction` contract, including hover, pressed, focus, and disabled states.
- Project the semantic role through presentation-only `objectName` bindings in `settings_dialog.py` and `workspace_search_dialog.py`.
- Keep the `QDialogButtonBox` Save/Cancel construction, dialog acceptance/rejection signals, search signal, cancellation signal, locale refresh, and application/service boundaries unchanged.
- Keep FindBar, plugin dialogs, and the broader status/activity hierarchy out of this slice; they require separate bounded acceptance decisions.
- No new ADR is required: this is a direct application of the existing localization/appearance boundary and centralized QSS decision in ADR-0035.

## Fixed-role input and ownership

| Role | Agent | Contribution | Disposition |
|---|---|---|---|
| Architect | parent | Integrated the design, edited the two presentation files, reviewed the final diff, and owns verification/handoff | accepted |
| Project Manager | Wegener / Luna | Proposed a broader activity-document/status-rail slice | considered; not selected for UI-12 |
| Product | Singer / Luna | Recommended a unified highlight anchor and bounded UI-12 acceptance | incorporated |
| Developer 1 | Pauli / Luna | Returned a SerialForge answer unrelated to this repository | not used; no source authority |
| Developer 2 | Socrates / Luna | Identified the two precise primary-action bindings and the existing QSS contract | selected implementation design |
| QA | Turing / Luna | Supplied static verification matrix and no-launch boundaries | incorporated |

The independent design reviewer Faraday / Luna did not return a conclusion within the bounded wait and was closed; no child PASS is claimed for that review. The independent code reviewer Arendt / Luna returned `PASS` for static review with runtime visual verification explicitly unrun.

## Simplification assessment

The smallest complete implementation is two direct object-name bindings. The `None` guard around the `QDialogButtonBox` Save lookup preserves safety at the Qt API boundary; extracting a helper or duplicating QSS would add indirection without reducing risk. No behavior-preserving simplification beyond this was identified.

Arendt's independent review noted that the existing later `setText()` calls assume the standard Save/Cancel buttons are present. That is an existing construction contract, low risk under the current `QDialogButtonBox` setup, and outside this presentation-only slice; no unrelated refactor was made.

## Public source applicability

This change touches Python/PyQt6 presentation code only. No embedded C/C++, MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, bootloader, Flash/NVM, power, or motor-control code was modified. Therefore the embedded vendor-source workflow and manufacturer-requirement applicability are `N/A`; no MISRA, ISO 26262, automotive, or certification claim is made.

## Independent review result

Arendt / Luna static review: `PASS`.

Evidence from the reviewer:

- The Settings Save button is obtained from the owning `QDialogButtonBox` immediately after construction and receives only a QSS semantic name.
- The workspace-search button is a directly owned `QPushButton`; the binding does not alter signals, state, or search logic.
- The existing `primaryAction` normal/hover/pressed/focus/disabled selectors cover the new bindings.
- Risks retained: runtime Qt/native style rendering, DPI/font metrics, accessibility contrast/screen-reader behavior, and the global semantic-name convention.

## Authorized verification

Static source checks completed for this slice:

- `uv run python -m compileall -q src\quillforge\presentation` — PASS.
- `uv run ruff check src\quillforge\presentation\settings_dialog.py src\quillforge\presentation\workspace_search_dialog.py` — PASS.
- `uv run ruff format --check src\quillforge\presentation\settings_dialog.py src\quillforge\presentation\workspace_search_dialog.py` — PASS.
- Source probe confirmed both bindings and the centralized `QPushButton#primaryAction` selector — PASS.

Package identity is now recorded as root/dist SHA-256 `BB177F29510299E30714CDA82B6F63248D2CE5FF03523CE75F2A7940073D235B`, `38365699` bytes, with source snapshot `tree-sha256:c0e1348097ec7f31bc871d1641a79da5a53cf5249dc89bd8484d93c86870d253`. Repository handoff validation passed. `scripts/verify_release_handoff.ps1` returned the expected non-zero `NO-GO` with exactly `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`; ten external gates remain open and are not altered by UI-12.

## Limits

- The project no-launch policy prohibits QApplication/Qt startup, opening dialogs, screenshots, and interactive visual acceptance in this checkout.
- QSS specificity against the platform style, native button metrics, installed fonts, DPI, formal contrast, screen-reader output, and cross-machine rendering remain open.
- Release readiness remains `NO-GO` until the existing stale runtime-report failures and external release gates are resolved; this UI slice does not close D7/D8/D9 external conditions.
