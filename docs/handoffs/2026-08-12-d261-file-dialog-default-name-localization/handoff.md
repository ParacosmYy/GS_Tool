# Handoff: 2026-08-12-d261-file-dialog-default-name-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d261-file-dialog-default-name-localization` |
| Delivery / slice | `D261 / ARCH-239 File-dialog default-name localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

When Save As is opened without a current document, the default filename is
`Untitled.txt` in English and `未命名文档.txt` in Simplified Chinese. Existing
document paths continue to be passed through unchanged.

## Scope and boundaries

The change reuses the existing `document.untitled` presentation catalog value
at the existing `FileDialogSurface.choose_save_path` boundary. The `.txt`
extension, dialog title/filter, selected-path return contract, document
model, filesystem policy, persistence, and startup remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Hubble the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Heisenberg the 7th / Luna max` independent-review window: `NO_CONCLUSION`;
  no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/file_dialog_surface.py` — localize only the
  no-current-document fallback name.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The fallback is display-only and remains owned by the presentation adapter.
No EXE/Qt launch, native dialog, registry, installer, updater, worktree, or
test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Default-name localization probe | `D261-SAVE-NAME-LOCALIZATION=PASS locales=2 extension=.txt` |
| Save-name wiring probe | `D261-SAVE-NAME-WIRING=PASS current_path_preserved=1` |
| Python compile | `D261-COMPILEALL=PASS` |
| Ruff | `D261-RUFF=PASS` |
| Ruff format | `D261-FORMAT=PASS` |
| Source startup diagnostic | `D261-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `69508BF3FD81665B03903CCA3D99D2D25A26F146B29DF5869265C3337EB71203` |
| PS7 package build | `PASS`, SHA `3EFADED47617CDD9AE3C7D486F2865F5E92C21A9D5ED05D050CD6996B785B097` |
| Package identity | root/dist match, 38,582,445 bytes |
| Source revision | `tree-sha256:d2d9ea618764f9c742ae5df1a1e9b535625a29364bb8719d0aea431052329aaf` |
| Frozen runtime dependencies | `D261-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D261-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D261-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |
| Project checks | `D261-CHECK-PS51=PASS`, `D261-CHECK-PS7=PASS` |
| Release verifier | `D261-RELEASE-VERIFY=EXPECTED-NO-GO open_gates=10` |

## Unrun checks and reason

Native EXE/Qt startup, native Save As dialog, desktop association launch,
clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probes verify catalog projection and current-path preservation but
not native dialog rendering, Windows filename metrics, DPI behavior, or GUI
startup. The current release dossier remains `no-go` because artifact-bound
startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S309`, `ARCH-239`, `D261-SAVE-NAME-LOCALIZATION=PASS locales=2`,
`D261-SIMPLIFICATION-ASSESSMENT=PASS`,
`D261-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D261-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `3EFADED47617CDD9AE3C7D486F2865F5E92C21A9D5ED05D050CD6996B785B097`
- Bytes: `38,582,445`
- Source revision: `tree-sha256:d2d9ea618764f9c742ae5df1a1e9b535625a29364bb8719d0aea431052329aaf`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The localized default filename is statically and
package verified; native dialog rendering and remaining release gates remain
open.
