# D262 parent review — early startup fallback localization

## Decision

`PASS` with operational limits.

## Findings

- The change stays in the pre-Qt `__main__` entry boundary and does not import
  the presentation catalog or construct Qt objects.
- Chinese-like environment/OS locale hints select the supported `zh-CN`
  fallback; unknown or unavailable hints retain the original English text.
- The original exception type/message, diagnostic path and log contents,
  MessageBox flags, stderr fail-open path, and exit code `1` remain unchanged.
- The localized title is computed inside the existing native-call guard, so
  locale-projection failure cannot replace the original failure handling.

## Evidence

- `D262-STARTUP-FALLBACK-LOCALIZATION=PASS current_locale=zh-CN qt_free=1`
- `D262-STARTUP-CONTRACT-SOURCE=PASS exit_code=1 diagnostic_path_preserved=1`
- `D262-COMPILEALL=PASS`, `D262-RUFF=PASS`, `D262-FORMAT=PASS`
- `D262-SOURCE-DIAGNOSTIC-EXIT=0`
- `D262-ARCHIVE-RUNTIME-DEPENDENCIES=PASS required=7 outer_entries=166`
- `D262-ARCHIVE-PYZ=PASS entries=261 modules=4`
- `D262-PYINSTALLER-WARNING-SCOPE=PASS lines=29`
- `D262-PACKAGE-IDENTITY=PASS`; root/dist match, 38,583,149 bytes

## Simplification assessment

`D262-SIMPLIFICATION-ASSESSMENT=PASS`: a small local two-locale resolver is
smaller and safer than loading settings or the Qt catalog during an early
startup exception. Existing English text remains the fallback and no new
startup service or policy owner is introduced.

## Review roles

- `Carson the 7th / Luna max` architecture window: `NO_CONCLUSION` after a
  bounded wait and closure.
- `Arendt the 7th / Luna max` independent-review window: `NO_CONCLUSION`
  after a bounded wait and closure; no independent PASS is claimed.
- Parent review: `PASS`.
- Simplification: `PASS`.

## Limits

No EXE/Qt launch, native MessageBox invocation, registry, installer, updater,
unit-test asset, mock, fixture, harness, worktree, or clean-machine run was
performed.

