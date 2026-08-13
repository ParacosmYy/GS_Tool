# Handoff: 2026-08-12-d259-message-dialog-button-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d259-message-dialog-button-localization` |
| Delivery / slice | `D259 / ARCH-237 Message-dialog standard button localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

Save, Discard, Cancel, and OK in the common message dialogs now use the
selected application locale instead of depending on the host Qt/system
locale. The Chinese shell gets explicit Chinese labels while English retains
the standard English labels.

## Scope and boundaries

`MessageSurface` adds one private button-text helper and uses four catalog
keys after standard-button creation. Standard-button return values, default
button, semantic roles, dialog modality, recovery prompt, settings dialog,
and startup behavior remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Carver the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Wegener the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add four dialog button catalog
  entries.
- `src/quillforge/presentation/message_surface.py` — explicitly project
  standard button text after creation.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The application locale owns the visible labels, while Qt’s standard-button
enum remains the interaction contract. No global Qt translator, application
policy, dialog result semantics, EXE/Qt launch, registry, installer, updater,
worktree, or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Dialog button catalog probe | `D259-DIALOG-BUTTON-CATALOG=PASS keys=4` |
| Dialog button wiring/order probe | `D259-DIALOG-BUTTON-WIRING=PASS standard_sets=3 projected_buttons=5` |
| Python compile | `D259-COMPILEALL=PASS` |
| Ruff | `D259-RUFF=PASS` |
| Ruff format | `D259-FORMAT=PASS` |
| Source startup diagnostic | `D259-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `64EF254363261504B8C049E47724E5435906DC20E2BE27BE8D6C32C52AE3005B` |
| PS7 package build | `PASS`, SHA `D5D37997111188E0769968004789782A4A4548AECE002D013D29FF0E1E284612` |
| Package identity | root/dist match, 38,583,331 bytes |
| Source revision | `tree-sha256:52c5e707ab3c1b5da71b771b9b8fd060a3252c8fb956af490423e6e5be03bfae` |
| Frozen runtime dependencies | `D259-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D259-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D259-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association
launch, clean-machine startup, signing, installer, updater/rollback,
registry, cross-machine, support, and release-owner checks remain unrun under
the permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The static catalog and source-order checks verify the projection boundary but
not native button rendering, platform-specific Qt standard-button creation, or
GUI startup. The current release dossier remains `no-go` because
artifact-bound startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S307`, `ARCH-237`, `D259-DIALOG-BUTTON-CATALOG=PASS keys=4`,
`D259-SIMPLIFICATION-ASSESSMENT=PASS`,
`D259-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D259-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `D5D37997111188E0769968004789782A4A4548AECE002D013D29FF0E1E284612`
- Bytes: `38,583,331`
- Source revision: `tree-sha256:52c5e707ab3c1b5da71b771b9b8fd060a3252c8fb956af490423e6e5be03bfae`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Common message-dialog button labels are statically and
package verified; native dialog rendering and remaining release gates remain
open.
