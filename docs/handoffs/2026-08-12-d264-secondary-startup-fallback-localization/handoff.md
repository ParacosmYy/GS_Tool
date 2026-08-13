# Handoff: 2026-08-12-d264-secondary-startup-fallback-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d264-secondary-startup-fallback-localization` |
| Delivery / slice | `D264 / ARCH-242 Secondary startup fallback localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

## User outcome

The last-resort pre-Qt startup error fallback now remains Chinese on a Chinese
system even if error-detail construction fails. If the locale probe also fails,
the original English final fallback remains available.

## Scope and boundaries

The change adds only a nested fail-open locale guard to `__main__.py`. It does
not alter the original exception detail, diagnostic path/log, MessageBox flags,
stderr fallback, process exit code, normal startup, settings, or Qt loading.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- `Huygens the 7th / Luna max` architecture window: `NO_CONCLUSION`.
- `Locke the 7th / Luna max` independent-review window: `NO_CONCLUSION`;
  no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/__main__.py` — localize the secondary fail-open fallback.
- ADR, review, acceptance/delivery/handoff, roadmap, architecture, release,
  and task records — evidence binding.

## Decisions and constraints

The English literal remains the final fallback if locale classification fails.
No EXE/Qt launch, native MessageBox, registry, installer, updater, worktree,
or test-only asset is authorized here.

## Verification commands and results

| Evidence | Result |
|---|---|
| Nested fail-open probe | `D264-STARTUP-FAILOPEN-LOCALIZATION=PASS nested_locale_failure=covered final_english_fallback=preserved` |
| Startup contract source probe | `D264-STARTUP-CONTRACT-SOURCE=PASS qt_free=1 exit_code=1` |
| Python compile | `D264-COMPILEALL=PASS` |
| Ruff | `D264-RUFF=PASS` |
| Ruff format | `D264-FORMAT=PASS` |
| Source startup diagnostic | `D264-SOURCE-DIAGNOSTIC-EXIT=0`, no GUI/QApplication launched |
| PE static preflight | `D264-PE-HEADER=PASS machine=AMD64 subsystem=WINDOWS_GUI imports=5` |
| PS5.1 package build | `PASS`, SHA `A5D8C4EA88B47BE3290636A1FC826AFA04B4F66BF2CFE31F2FB91343DC551275` |
| PS7 package build | `PASS`, SHA `4B64D8CEE650FD1CCB38C56E974146FFF0714E4C87B07FD62B85B6D01AE86542` |
| Package identity | root/dist match, 38,582,328 bytes |
| Source revision | `tree-sha256:f947cd8b908e79c822c868690cbc02d3afa58d5b5194c4d4188594c6e9d96383` |
| Frozen runtime dependencies | `D264-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166` |
| Embedded PYZ modules | `D264-ARCHIVE-PYZ=PASS entries=261 required_modules=3` |
| PyInstaller warnings | `D264-PYINSTALLER-WARNING-SCOPE=PASS lines=29` |
| Project checks | `D264-CHECK-PS51=PASS`, `D264-CHECK-PS7=PASS` |
| Release verifier | `D264-RELEASE-VERIFY=EXPECTED-NO-GO open_gates=10` |

## Unrun checks and reason

Native EXE startup, actual Windows MessageBox rendering, desktop association
launch, clean-machine startup, signing, installer, updater/rollback, registry,
cross-machine, support, and release-owner checks remain unrun under the
permanent no-launch and non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The source probe verifies the nested fail-open shape but not native Windows
rendering, OS-locale matrix coverage, DPI behavior, or GUI startup. The current
release dossier remains `no-go` because artifact-bound startup/performance
reports and enterprise gates are open.

## Acceptance and evidence IDs

`S312`, `ARCH-242`,
`D264-STARTUP-FAILOPEN-LOCALIZATION=PASS nested_locale_failure=covered final_english_fallback=preserved`,
`D264-SIMPLIFICATION-ASSESSMENT=PASS`,
`D264-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D264-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns any future explicitly authorized native startup and
clean-machine evidence. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `4B64D8CEE650FD1CCB38C56E974146FFF0714E4C87B07FD62B85B6D01AE86542`
- Bytes: `38,582,328`
- Source revision: `tree-sha256:f947cd8b908e79c822c868690cbc02d3afa58d5b5194c4d4188594c6e9d96383`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The secondary startup fallback is statically, PE, and
package verified; native startup rendering and remaining release gates remain
open.
