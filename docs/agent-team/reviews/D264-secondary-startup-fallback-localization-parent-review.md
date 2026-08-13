# D264 parent review — secondary startup fallback localization

## Decision

`PASS` with operational limits.

## Findings

- The change is confined to the existing pre-Qt fail-open handler in
  `__main__.py`.
- The secondary fallback selects Chinese labels when the guarded locale probe
  succeeds and retains the exact prior English message when it does not.
- The nested guard prevents locale-probe failure from escaping the startup
  error path; MessageBox/stderr behavior, diagnostic data, and exit code are
  unchanged.

## Evidence

- `D264-STARTUP-FAILOPEN-LOCALIZATION=PASS nested_locale_failure=covered final_english_fallback=preserved`
- `D264-STARTUP-CONTRACT-SOURCE=PASS qt_free=1 exit_code=1`
- `D264-COMPILEALL=PASS`, `D264-RUFF=PASS`, `D264-FORMAT=PASS`
- `D264-SOURCE-DIAGNOSTIC-EXIT=0`
- `D264-PE-HEADER=PASS machine=AMD64 subsystem=WINDOWS_GUI imports=5`
- `D264-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166`
- `D264-ARCHIVE-PYZ=PASS entries=261 required_modules=3`
- `D264-PYINSTALLER-WARNING-SCOPE=PASS lines=29`
- `D264-PACKAGE-IDENTITY=PASS`; root/dist match, 38,582,328 bytes

## Simplification assessment

`D264-SIMPLIFICATION-ASSESSMENT=PASS`: the nested guard is the smallest way to
extend localization without weakening the final English fail-open fallback or
adding a pre-Qt dependency. No new policy owner or error channel is added.

## Review roles

- `Huygens the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Locke the 7th / Luna max` independent-review window: `NO_CONCLUSION` after
  a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native MessageBox invocation, registry, installer, updater,
unit-test asset, mock, fixture, harness, worktree, or clean-machine run was
performed.

