# Handoff: 2026-08-10-d9-ui-12-action-hierarchy

| Field | Value |
|---|---|
| ID | `2026-08-10-d9-ui-12-action-hierarchy` |
| Delivery / slice | `D9 / UI-12 dialog primary-action hierarchy` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T04:01:40+08:00` |

## User outcome

Settings Save and workspace-search Search are now visibly primary through the existing Sakura Pop/legacy centralized QSS contract. Cancel and Close remain secondary, with no change to dialog signals, search behavior, settings persistence, or application ownership.

## Scope and boundaries

### In scope

- Assign the existing `primaryAction` semantic object name to Settings Save.
- Assign the existing `primaryAction` semantic object name to workspace-search Search.
- Record the UI-12 acceptance, independent review, simplification assessment, and package evidence.

### Out of scope

- No new theme engine, color token, widget behavior, command routing, async flow, or domain/application change.
- No FindBar/plugin-dialog/activity-rail redesign.
- No Qt application launch, screenshot, interactive visual acceptance, test-only asset, or target deployment.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | parent | Integration, final review, verification, and handoff decision |
| Project Manager | Wegener / Luna | Plan, dependencies, risks, and status input |
| Product | Singer / Luna | User outcome and acceptance input |
| Developer 1 | Pauli / Luna | Domain/application/infrastructure review input; unrelated response not used |
| Developer 2 | Socrates / Luna | Presentation/integration design input |
| QA | Turing / Luna | Read-only static verification and unrun evidence |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — marks the standard Save button as `primaryAction` after `QDialogButtonBox` construction, guarded for the Qt lookup boundary.
- `src/quillforge/presentation/workspace_search_dialog.py` — marks the Search button as `primaryAction` immediately after construction.
- `docs/agent-team/acceptance.json` — adds `D9-AC10` and `S37`.
- `docs/agent-team/delivery-register.json` — registers UI-12 and the new acceptance IDs.
- `docs/agent-team/reviews/D9-UI-12-action-hierarchy-parent-review.md` — records architecture, role, independent review, simplification, and limits.
- `docs/handoffs/index.json` — makes this handoff the latest entry.

## Decisions and constraints

- The canonical style remains `src/quillforge/presentation/theme.py`; UI-12 uses the existing `QPushButton#primaryAction` selectors rather than adding duplicate QSS.
- Shared checkout writer: Architect, with production source limited to the two presentation files and documentation limited to this handoff/review/ledger update.
- Runtime launch policy: launch is prohibited by the project policy, so Qt startup, screenshots, and interactive visual acceptance are intentionally not run.
- No embedded C/C++ or firmware scope applies; public vendor-source applicability is `N/A` and no compliance claim is made.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src\quillforge\presentation` | PASS | Completed before documentation integration. |
| `uv run ruff check src\quillforge\presentation\settings_dialog.py src\quillforge\presentation\workspace_search_dialog.py` | PASS | Changed presentation files clean. |
| `uv run ruff format --check src\quillforge\presentation\settings_dialog.py src\quillforge\presentation\workspace_search_dialog.py` | PASS | Formatting unchanged. |
| Static source probe | PASS | Both object-name bindings and the existing centralized selector found. |
| Arendt / Luna independent static review | PASS | Runtime visual acceptance explicitly unrun. |
| `uv run python -m compileall -q src\quillforge` | PASS | Whole-source compile completed. |
| `uv run ruff check src\quillforge` | PASS | All checks passed. |
| `uv run ruff format --check src\quillforge` | PASS | 62 files already formatted. |
| JSON parse of acceptance/register/index/manifest/dossier | PASS | Final parse is rerun after release verification. |
| `scripts/verify_handoff.ps1` | PASS | Handoff structure/traceability gate passed; rerun after final ledger update. |
| `scripts/check.ps1` | PASS | Repository static gate passed; rerun after final ledger update. |
| `scripts/package.ps1` via PowerShell Core | PASS | PyInstaller/package completed; Windows PowerShell invocation hit an executor API incompatibility and was not used for the successful evidence. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO (exit 1) | Dossier refreshed with current artifact; exact mechanical failures remain `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`; 10 external gates remain open. |

## Unrun checks and reason

- QApplication/Qt startup, dialog interaction, screenshots, and runtime visual acceptance — prohibited by the current project no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created, modified, or run under the project R&D constraints.
- Hardware, deployment, signing, installer, clean-machine, and cross-machine checks — outside this local static/source/package slice and not authorized.

## Known risks and limits

- Actual QSS/native-style specificity, DPI, installed-font metrics, formal contrast, screen-reader output, and cross-machine rendering remain unverified.
- `primaryAction` is a global QSS semantic name; it is currently presentation-only and has no lookup/behavior meaning.
- Release status remains `NO-GO` until the existing three stale runtime-report mechanical failures and ten external release gates are resolved.

## Acceptance and evidence IDs

- Acceptance: `D9-AC10`, `S37`.
- Evidence: `src/quillforge/presentation/theme.py`, `src/quillforge/presentation/settings_dialog.py`, `src/quillforge/presentation/workspace_search_dialog.py`, `docs/agent-team/reviews/D9-UI-12-action-hierarchy-parent-review.md`, `scripts/verify_handoff.ps1`, `scripts/check.ps1`, `scripts/package.ps1`, `dist/QuillForge.release.json`.

## Next owner and next action

- Owner: `architect`.
- Action: Keep D7/D8/external release gates open until an authorized operator supplies the missing runtime, clean-machine, legal, signing, installer/update, and cross-machine evidence.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- SHA-256 / size: `BB177F29510299E30714CDA82B6F63248D2CE5FF03523CE75F2A7940073D235B` / `38365699` bytes for both root and dist copies; source `tree-sha256:c0e1348097ec7f31bc871d1641a79da5a53cf5249dc89bd8484d93c86870d253`.
- Packaging note: portable package rebuilt through PowerShell Core because the Windows PowerShell executor did not expose `System.IO.Path.GetRelativePath`; package output itself completed successfully. This is not a release approval.

## Disposition

`accepted-with-limits`: the bounded primary-action hierarchy is implemented, independently statically reviewed, packaged, and recorded. Static/runtime-policy evidence is complete for this slice; runtime visual and external release conditions remain open.
