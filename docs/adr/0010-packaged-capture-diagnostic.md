# ADR-0010: Explicit packaged capture diagnostic

## Status

Accepted for implementation as a non-default diagnostic path.

## Context

D7.3 has a reproducible source/offscreen recovery-capture baseline, but that
baseline cannot prove the behavior of the bundled EXE. The normal editor must
not run a benchmark, write diagnostics, or change startup behavior merely
because it is packaged.

## Decision

1. Add an explicit `--diagnose-capture` command-line path to the application.
2. The diagnostic creates a QScintilla adapter, captures a bounded synthetic
   workload through the production `TextCaptureSession`, publishes through the
   production bounded recovery channel to the atomic recovery writer, yields
   through real Qt timers, and records elapsed time, heartbeat gaps, capture
   completeness, queue occupancy, commit state, and Windows process-memory
   samples.
3. The diagnostic writes one JSON report to the caller-selected path through an
   atomic same-directory replacement. It does not open a normal main window,
   inspect user files, discover plugins, use the network, or run in normal
   startup.
4. `scripts/measure_packaged.ps1` verifies the root test copy against
   `dist/QuillForge.release.json`, launches the exact root EXE, repeats the
   diagnostic three times, attaches the EXE hash/size and explicit unrun list,
   and stores the resulting evidence outside the executable.
5. Packaged diagnostic evidence is path-specific. It does not establish clean
   machine behavior, arbitrary large-file support, disk-full behavior,
   hard-power durability, or a product memory ceiling.

## Consequences

- D7.3 can compare source/offscreen and packaged-EXE execution without
  conflating the two paths.
- The normal composition root remains unchanged for ordinary launches.
- The diagnostic is an observability/release tool, not a user editing feature;
  it remains explicitly gated and documented.
- The packaged diagnostic now exercises the bounded channel and atomic commit
  path, but it remains an evidence tool; it does not establish a product
  memory ceiling or remove all D7.2 larger-workload and clean-environment risks.
