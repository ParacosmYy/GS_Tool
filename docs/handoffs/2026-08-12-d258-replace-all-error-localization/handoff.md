# Handoff: 2026-08-12-d258-replace-all-error-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d258-replace-all-error-localization` |
| Delivery / slice | `D258 / ARCH-236 Replace All error localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The Chinese shell now localizes the Replace All match-limit exception,
incomplete text-capture error, and rollback-failure errors. Existing status
counts, English output, and unknown diagnostic details remain preserved.

## Scope and boundaries

Only `presentation.i18n` changed. The existing localizer now handles one exact
dynamic error shape and three exact exception forms. Editor transaction state,
rollback behavior, QScintilla integration, error propagation, and startup
behavior remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Socrates the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Hooke the 7th / Luna max` independent-review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add bounded Replace All and text
  capture error mappings in the existing presentation localizer.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The dynamic message is matched with an anchored regular expression and its
counter is retained. Unknown messages remain unchanged. No EXE/Qt launch,
native dialog, registry, installer, updater, worktree, or test-only asset is
authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Replace All/error localization probe | `D258-REPLACE-LOCALIZATION=PASS cases=7` |
| Catalog key parity | `D258-CATALOG-KEYS=PASS keys=264` |
| Python compile | `D258-COMPILEALL=PASS` |
| Ruff | `D258-RUFF=PASS` |
| Ruff format | `D258-FORMAT=PASS` |
| Source startup diagnostic | `D258-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `7A3F6A7508ABD1C8D5DB469531D2DE9B6FF59CF5BBE6A890E25239C79306C4CF` |
| PS7 package build | `PASS`, SHA `4EC3BF4BB2AC6F5B3C163EEE268B1CC629AEF8A0ADF4EC0BC185626B670C10CB` |
| Package identity | root/dist match, 38,582,455 bytes |
| Source revision | `tree-sha256:52df157559c86e19a9e8541f1feef0eeb402045ac52a1a20331fe7f3a5844d52` |
| Frozen runtime dependencies | `D258-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D258-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D258-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, desktop association
launch, clean-machine startup, signing, installer, updater/rollback,
registry, cross-machine, support, and release-owner checks remain unrun under
the permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probe verifies translation and fallback behavior but not native
rendering, runtime error timing, rollback execution, or GUI startup. The
current release dossier remains `no-go` because artifact-bound
startup/performance reports and enterprise gates are open.

## Acceptance and evidence IDs

`S306`, `ARCH-236`, `D258-REPLACE-LOCALIZATION=PASS cases=7`,
`D258-SIMPLIFICATION-ASSESSMENT=PASS`,
`D258-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D258-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `4EC3BF4BB2AC6F5B3C163EEE268B1CC629AEF8A0ADF4EC0BC185626B670C10CB`
- Bytes: `38,582,455`
- Source revision: `tree-sha256:52df157559c86e19a9e8541f1feef0eeb402045ac52a1a20331fe7f3a5844d52`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. Replace All error strings are statically and package
verified; native startup and remaining release gates remain open.
