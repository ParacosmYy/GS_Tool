# D299 independent review — frozen Qt plugin root selection

## Scope

An independent Luna/max reviewer was asked to inspect the qwindows-bearing
root preference, Qt6/Qt fallback behavior, reuse by configuration and
diagnostics, D297 integration, and possible startup regressions.

## Initial result

`REVISE`. The first independent reviewer confirmed the runtime selector
behavior but identified that the original static audit did not constrain
Qt6-to-legacy order, `is_file()` precedence, or configuration/diagnostic
reuse strongly enough. It also inspected the package before the subsequent
rebuild and therefore correctly rejected stale artifact evidence for D299.

## Follow-up result

The final Luna/max review was started after the AST-backed audit revision and
the final rebuild. It returned `PASS` for the current static contract and
artifact identity only. It does not approve native runtime, clean-machine, or
physical split-layout behavior.

## Parent evidence retained

The parent and final independent review confirmed that the current Qt6 root remains selected, a
complete legacy root wins when a Qt6 directory lacks `qwindows.dll`, and both
plugin configuration and diagnostic reporting share one selector. The revised
AST contract now guards those properties. Compile, lint, format, source
diagnostics, project checks, and the rebuilt package/archive inspection passed.

## Unrun and residual evidence

No physical split-layout bundle was generated, no native EXE/Qt launch or
clean-machine run was performed, and no unit tests, mocks, fixtures,
harnesses, signing, installer, updater, or release-owner evidence was run.
Windows DLL loader behavior remains outside the static/package evidence.

No embedded C/C++, MCU, RTOS, manufacturer requirement, MISRA, ISO 26262,
ASPICE, certification, or private ByteDance-standard claim applies.
