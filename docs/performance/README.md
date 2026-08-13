# Performance measurement contract

Run the reproducible local baseline with:

```powershell
.\scripts\measure.ps1
```

For the D7.3 repeatability gate, run:

```powershell
.\scripts\repeat_measure.ps1
```

For the packaged-EXE capture path, run:

```powershell
.\scripts\measure_packaged.ps1
```

The repeat command runs the source/offscreen baseline three times, requires a
stable artifact hash, requires full capture and round-trip results in every
run, and records the evidence under `docs/performance/`. It still does not
measure packaged-EXE capture performance or authorize a large-file support
claim.

The packaged command launches the exact root `QuillForge.exe` with the
explicit `--diagnose-capture` path three times, requires a stable EXE hash,
and records per-run latency, Qt heartbeat gaps, capture completeness, and
Windows process-memory samples. It verifies the root copy against
`dist/QuillForge.release.json` before running. Its default workload is
16,777,232 bytes.
This is packaged EXE plus Qt offscreen evidence on the current machine; it is
not clean-machine, interactive non-offscreen, disk-pressure, hard-power, or
product memory-ceiling evidence.

The D7.4 workspace-search package probe writes
`docs/performance/workspace-search-packaged-2026-08-09.json` through the
explicit Qt-free `--diagnose-workspace-search <root> <literal> --report <path>`
path. Its result is a bounded feature diagnostic, not a performance SLA or a
general large-file support claim. The JSON report includes bounded
`issue_records` with workspace-relative paths and an `issues_truncated` flag;
the report is regenerated after packaging and the surrounding release evidence
binds it to the current EXE artifact identity.

The current-machine interactive startup record is
`docs/performance/interactive-startup-2026-08-09.json`. It verifies that the
root EXE creates a real `QuillForge` window and remains alive for the configured
five-second threshold, then verifies exact-path process cleanup. It is a
current-developer-machine baseline only and does not satisfy the clean-machine
gate.

Regenerate that record reproducibly with:

```powershell
.\scripts\verify_interactive_startup.ps1 -ExecutablePath .\QuillForge.exe
```

The script binds the record to the exact artifact hash and window path. It is
still a current-machine record, not a clean-machine attestation.

For a portable startup preflight on another Windows x64 machine, run from the
handoff directory:

```powershell
.\scripts\verify_clean_machine.ps1 -ExecutablePath .\QuillForge.exe -EnvironmentLabel clean-windows-x64-<machine-id>
```

The script has no Python or Qt dependency. It records the EXE identity, Windows
environment, exact process path, `QuillForge` window title, configured lifetime,
and cleanup result. Its report still requires QA/Architect review before being
classified as clean-machine evidence; the current checked-in report explicitly
sets `clean_machine` to `false`. To make an explicit operator attestation on an
independent machine, add `-CleanMachineAttested`; the script rejects that switch
unless the label begins with `clean-windows-x64-`.

The command creates only a uniquely named temporary directory, measures representative file I/O, a size/encoding/EOL/long-line recovery-capture matrix, current-document literal search/replace, cancellation/rollback, match-limit safety, stale-capture discard, recovery snapshot writing, a mid-stream recovery-write failure probe, and a Qt timer/process-memory probe, then removes the directory. It prints JSON containing input sizes, timings, Python allocation peaks, platform identity, the current root EXE identity when present, claims, and unrun limits. Its execution path is still source Python with Qt offscreen; the EXE identity is provenance, not an EXE performance measurement.

The output is a machine baseline, not a support claim. A release claim requires repeated measurements on the supported Windows architecture, clean-machine evidence, native Qt/QScintilla memory accounting, permission/disk-pressure behavior, and an agreed workload set. D7 remains incomplete until those limits are interpreted by the Architect and QA.

## Active-document operation guard

The current product policy is intentionally explicit and injectable:

- Replace All is limited to 10,000 literal matches.
- Counting and replacement run as separate QScintilla-owned phases.
- Replacement yields to the Qt event loop after an 8 ms or 256-item slice, whichever comes first.
- The Find Bar exposes cancellation; cancellation and operation errors roll back the current undo transaction.
- When the match limit is exceeded, no document text is changed.

These are responsiveness and data-safety guards, not a large-file support claim. The D7.2 capture path reads position-aligned chunks in the UI thread over an 8 ms/512-chunk budget, with at most 16,384 Scintilla characters per chunk; it offers them through the default bounded 64-chunk/1 MiB UTF-8 channel to the worker, where the chunk-capable store encodes JSON directly into the atomic temporary file. The UI retains only the active candidate chunk in this default composition; a single chunk is still one adapter read and the bound is not a product native-memory ceiling.
