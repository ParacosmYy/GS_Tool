# Handoff: 2026-08-12-d251-workspace-search-diagnostic-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d251-workspace-search-diagnostic-locale-refresh` |
| Delivery / slice | `D251 / ARCH-229 Workspace-search diagnostic locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Workspace-search diagnostic rows now display localized stable reasons after a
language change. Relative paths and provider-specific exception/detail text
remain visible, and the diagnostic list keeps its current expansion state.

## Scope and boundaries

The change is limited to `WorkspaceSearchDialog` and the shared presentation
i18n catalog. It retains the latest immutable result for re-projection,
translates only known provider reasons/prefixes, and leaves search services,
provider behavior, result counts, cancellation, and application policy alone.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Avicenna the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Newton the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/workspace_search_dialog.py` — retain result
  source and reproject diagnostic rows on locale changes.
- `src/quillforge/presentation/i18n.py` — map stable search diagnostic reasons
  and prefixes for zh-CN while preserving English.
- ADR, review records, acceptance/delivery/handoff records, roadmap,
  architecture, release handoff, and task checklists — evidence binding.

## Decisions and constraints

Only source data is retained; localized text is never written back to an
application result. No EXE/Qt launch, native dialog, registry, installer,
updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Diagnostic source retention | `D251-DIAGNOSTIC-SOURCE-RETENTION=PASS` |
| Diagnostic locale refresh | `D251-DIAGNOSTIC-LOCALE-REFRESH=PASS` |
| Prefix/reason catalog | `D251-DIAGNOSTIC-PREFIX-CATALOG=PASS` |
| Expansion preservation | `D251-DIAGNOSTIC-EXPANSION-PRESERVATION=PASS` |
| Python compile | `D251-COMPILEALL=PASS` |
| Ruff | `D251-RUFF=PASS` |
| Ruff format | `D251-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `805101DA9B6C5F06D981F76BC6E8535AEF91D4BEAF2CBC3A39800149618611F6` |
| PS7 package build | `PASS`, SHA `31788CA4CD50816F920F2AC9ED310CC67F605B489122918B56EFFE9AB684734E` |
| Package identity | root/dist match, 38,579,914 bytes |
| Source revision | `tree-sha256:c19a6dba52e3d85f9cbb922d89a8c2983d883bed7289d91ba1ea40da1df543db` |
| Frozen archive | `D251-ARCHIVE-OUTER=PASS`, outer=166, inner=261; entry/i18n/search/PyQt6/qwindows present |
| PyInstaller warnings | `D251-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, cross-machine, support, and release-owner checks
remain unrun under the permanent no-launch and non-destructive policy. No
unit-test assets were created or run.

## Known risks and limits

Only known stable provider reasons are translated. Unknown reasons remain
unchanged with their diagnostic detail. Static and archive evidence cannot
prove native rendering or Windows startup.

## Acceptance and evidence IDs

`S299`, `ARCH-229`, `D251-DIAGNOSTIC-SOURCE-RETENTION=PASS`,
`D251-DIAGNOSTIC-LOCALE-REFRESH=PASS`,
`D251-DIAGNOSTIC-PREFIX-CATALOG=PASS`,
`D251-SIMPLIFICATION-ASSESSMENT=PASS`,
`D251-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D251-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup, native
dialog, and clean-machine evidence. This handoff does not authorize launching
the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `31788CA4CD50816F920F2AC9ED310CC67F605B489122918B56EFFE9AB684734E`
- Bytes: `38,579,914`
- Source revision: `tree-sha256:c19a6dba52e3d85f9cbb922d89a8c2983d883bed7289d91ba1ea40da1df543db`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Workspace-search diagnostic locale refresh is packaged
and recorded; native startup, native dialogs, and remaining release gates
remain open.
