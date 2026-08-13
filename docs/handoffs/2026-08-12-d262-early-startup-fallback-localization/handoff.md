# Handoff: 2026-08-12-d262-early-startup-fallback-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d262-early-startup-fallback-localization` |
| Delivery / slice | `D262 / ARCH-240 Early startup fallback localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

If QuillForge fails before the normal Qt shell can load, a Chinese Windows
locale now receives a Chinese startup-error title/message and diagnostic-log
label. English and unknown locales retain the existing English fallback.

## Scope and boundaries

The change adds a Qt-free two-locale resolver to `__main__.py` and projects
only the fallback labels. Exception type/message, diagnostic path and log
format, MessageBox flags, stderr fallback, process exit code, normal startup,
settings, and the presentation catalog remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Carson the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Arendt the 7th / Luna max` independent-review window: `NO_CONCLUSION`;
  no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/__main__.py` — localize early startup fallback labels without
  adding Qt or settings dependencies.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The fallback must remain fail-open and diagnostic-first. No EXE/Qt launch,
native MessageBox, registry, installer, updater, worktree, or test-only asset
is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Locale/fallback source probe | `D262-STARTUP-FALLBACK-LOCALIZATION=PASS current_locale=zh-CN qt_free=1` |
| Startup contract source probe | `D262-STARTUP-CONTRACT-SOURCE=PASS exit_code=1 diagnostic_path_preserved=1` |
| Python compile | `D262-COMPILEALL=PASS` |
| Ruff | `D262-RUFF=PASS` |
| Ruff format | `D262-FORMAT=PASS` |
| Source startup diagnostic | `D262-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PS5.1 package build | `PASS`, SHA `3DC7FCD14B8A375BC40061165BC9282A9EA9935C8AA8A9A244CD0D729F8C8305` |
| PS7 package build | `PASS`, SHA `DC4B3DC264BA3EF93AC79653CFF1A255C7733AA3D7F6B4B01E3BB93F3E786CE6` |
| Package identity | root/dist match, 38,583,149 bytes |
| Source revision | `tree-sha256:bddd604faa64c5a7d1a2b70f5d9e7df18999deffcf6bea80e73e748f6023aa58` |
| Frozen runtime dependencies | `D262-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D262-ARCHIVE-PYZ=PASS entries=261 modules=4` |
| PyInstaller warnings | `D262-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |
| Project checks | `D262-CHECK-PS51=PASS`, `D262-CHECK-PS7=PASS` |
| Release verifier | `D262-RELEASE-VERIFY=EXPECTED-NO-GO open_gates=10` |

## Unrun checks and reason

Native EXE startup, actual Windows MessageBox rendering, desktop association
launch, clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probe verifies locale classification and preserved failure-path
shapes but not native Windows rendering, OS locale naming outside the two
supported product locales, DPI behavior, or GUI startup. The current release
dossier remains `no-go` because artifact-bound startup/performance reports and
enterprise gates are open.

## Acceptance and evidence IDs

`S310`, `ARCH-240`, `D262-STARTUP-FALLBACK-LOCALIZATION=PASS current_locale=zh-CN qt_free=1`,
`D262-SIMPLIFICATION-ASSESSMENT=PASS`,
`D262-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D262-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `DC4B3DC264BA3EF93AC79653CFF1A255C7733AA3D7F6B4B01E3BB93F3E786CE6`
- Bytes: `38,583,149`
- Source revision: `tree-sha256:bddd604faa64c5a7d1a2b70f5d9e7df18999deffcf6bea80e73e748f6023aa58`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The early fallback localization is statically and
package verified; native startup rendering and remaining release gates remain
open.
