# Handoff: 2026-08-12-d252-plugin-diagnostic-locale-refresh

| Field | Value |
|---|---|
| ID | `2026-08-12-d252-plugin-diagnostic-locale-refresh` |
| Delivery / slice | `D252 / ARCH-230 Plugin diagnostic locale refresh` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Plugin status and catalog tooltips now localize known runtime and manifest
diagnostic reasons when the interface is Chinese. Unknown provider text stays
visible, and changing locale refreshes the existing rows without changing
plugin governance behavior.

## Scope and boundaries

The change is limited to the two plugin dialog presentation endpoints and the
shared i18n catalog. Trust, approval, enablement, runtime registration,
external execution, scanning, permissions, and application ownership remain
unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Noether the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Turing the 7th / Luna max` independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/plugin_status_dialog.py` — localize visible
  runtime status errors at tooltip composition.
- `src/quillforge/presentation/plugin_catalog_dialog.py` — localize visible
  catalog entry reasons at tooltip composition.
- `src/quillforge/presentation/i18n.py` — add stable plugin policy/manifest
  reason and prefix mappings with unknown-text fallback.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

Only display text is projected. Raw plugin state and policy decisions are not
translated or rewritten. No EXE/Qt launch, native dialog, registry, installer,
updater, worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Status error routing | `D252-PLUGIN-STATUS-ERROR-ROUTING=PASS` |
| Catalog reason routing | `D252-PLUGIN-CATALOG-REASON-ROUTING=PASS` |
| Unknown text fallback | `D252-PLUGIN-UNKNOWN-TEXT-FALLBACK=PASS` |
| Localizer behavior | `D252-PLUGIN-LOCALIZER-BEHAVIOR=PASS cases=7` |
| Python compile | `D252-COMPILEALL=PASS` |
| Ruff | `D252-RUFF=PASS` |
| Ruff format | `D252-FORMAT=PASS` |
| PS5.1 package build | `PASS`, SHA `E84EB86D8DB673EA128BBCB825199D0CA80F5ECDD2579F539AD8C41B5D446D30` |
| PS7 package build | `PASS`, SHA `32B23AF303719818A5528F19778888D74058B9419713026EF144B30A91C72F58` |
| Package identity | root/dist match, 38,579,367 bytes |
| Source revision | `tree-sha256:9e8f908b85342e6b3c138056fb94f38477af6ea03b224a64b221d3497f036936` |
| Frozen archive | `D252-ARCHIVE-OUTER=PASS`, outer=166, inner=261; entry/i18n/plugin dialogs/PyQt6/qwindows present |
| PyInstaller warnings | `D252-PYINSTALLER-WARNING-SCOPE=PASS`, lines=25 |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association launch,
real filesystem permission timing, clean-machine startup, signing, installer,
updater/rollback, registry, cross-machine, support, and release-owner checks
remain unrun under the permanent no-launch and non-destructive policy. No
unit-test assets were created or run.

## Known risks and limits

Only known application-owned plugin reasons and prefixes are translated.
Unknown or future third-party diagnostics remain unchanged. Static and archive
evidence cannot prove native rendering, native locale switching, or startup.

## Acceptance and evidence IDs

`S300`, `ARCH-230`, `D252-PLUGIN-STATUS-ERROR-ROUTING=PASS`,
`D252-PLUGIN-CATALOG-REASON-ROUTING=PASS`,
`D252-PLUGIN-UNKNOWN-TEXT-FALLBACK=PASS`,
`D252-SIMPLIFICATION-ASSESSMENT=PASS`,
`D252-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D252-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup, native
dialog, and clean-machine evidence. This handoff does not authorize launching
the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `32B23AF303719818A5528F19778888D74058B9419713026EF144B30A91C72F58`
- Bytes: `38,579,367`
- Source revision: `tree-sha256:9e8f908b85342e6b3c138056fb94f38477af6ea03b224a64b221d3497f036936`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Plugin diagnostic locale refresh is packaged and
recorded; native startup, native dialogs, and remaining release gates remain
open.
