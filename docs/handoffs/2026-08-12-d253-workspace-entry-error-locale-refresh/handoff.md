# Handoff: 2026-08-12-d253-workspace-entry-error-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d253-workspace-entry-error-locale-refresh` |
| Delivery / slice | `D253 / ARCH-231 Workspace-entry error locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Disabled inaccessible workspace entries now show localized known provider
reasons in Chinese, and their tooltip updates when the language changes.
Unknown filesystem/provider text remains visible; files and folders keep their
existing activation behavior.

## Scope and boundaries

The change is limited to `WorkspacePanel` item projection and the shared i18n
catalog. It retains the raw entry error in presentation state, preserves the
disabled row, and leaves provider classification, directory bounds, sorting,
file/folder signals, navigation, and application ownership unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Hypatia the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Peirce the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/workspace_panel.py` — retain entry error source
  in a private item role and reproject disabled-row tooltips on locale changes.
- `src/quillforge/presentation/i18n.py` — add stable workspace provider reason
  mappings with unknown-text fallback.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

Only visible tooltip text is projected. Domain/provider error strings are not
rewritten, inaccessible rows remain disabled, and no EXE/Qt launch, native
dialog, registry, installer, updater, worktree, or test-only asset is
authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Error source retention | `D253-WORKSPACE-ERROR-SOURCE-RETENTION=PASS` |
| Tooltip localization | `D253-WORKSPACE-ERROR-TOOLTIP-LOCALIZATION=PASS` |
| Disabled-row locale refresh | `D253-WORKSPACE-DISABLED-LOCALE-REFRESH=PASS` |
| Unknown fallback | `D253-WORKSPACE-UNKNOWN-ERROR-FALLBACK=PASS` |
| Python compile | `D253-COMPILEALL=PASS` |
| Ruff | `D253-RUFF=PASS` |
| Ruff format | `D253-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `0169CD66F65C978B1B934C46EABCC76950ECE6F2C670885905D60645CD3D1256` |
| PS7 package build | `PASS`, SHA `9C10457042889B196BFD2F0BDAE26EA11FC8C9BA3A8D6CDCCE7AA04DED92033C` |
| Package identity | root/dist match, 38,580,637 bytes |
| Source revision | `tree-sha256:73dca3a0684d8c1d8b492e4b4e39db3a0ea126177445d3b16d5b2d52fbaa79b6` |
| Frozen archive | `D253-ARCHIVE-OUTER=PASS`, outer=166, inner=261; entry/i18n/workspace-panel/PyQt6/qwindows present |
| PyInstaller warnings | `D253-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, cross-machine, support, and release-owner checks
remain unrun under the permanent no-launch and non-destructive policy. No
unit-test assets were created or run.

## Known risks and limits

Only stable provider-owned workspace reasons are translated. Unknown or future
filesystem error strings remain unchanged. Static and archive evidence cannot
prove native rendering, native locale switching, file opening, or startup.

## Acceptance and evidence IDs

`S301`, `ARCH-231`, `D253-WORKSPACE-ERROR-SOURCE-RETENTION=PASS`,
`D253-WORKSPACE-ERROR-TOOLTIP-LOCALIZATION=PASS`,
`D253-WORKSPACE-DISABLED-LOCALE-REFRESH=PASS`,
`D253-SIMPLIFICATION-ASSESSMENT=PASS`,
`D253-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D253-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup, native
dialog, and clean-machine evidence. This handoff does not authorize launching
the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `9C10457042889B196BFD2F0BDAE26EA11FC8C9BA3A8D6CDCCE7AA04DED92033C`
- Bytes: `38,580,637`
- Source revision: `tree-sha256:73dca3a0684d8c1d8b492e4b4e39db3a0ea126177445d3b16d5b2d52fbaa79b6`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Workspace-entry error locale refresh is packaged and
recorded; native startup, native dialogs, file opening, and remaining release
gates remain open.
