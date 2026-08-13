# D200 parent review: package source-revision determinism

## Decision

`PASS` for the bounded packaging-provenance slice, accepted with explicit
runtime and external release limits.

## Review evidence

- The source inventory and per-file hashes are unchanged; only line ordering
  is moved to an ordinal .NET comparer.
- A read-only prototype reproduced the prior 11-line cross-shell difference
  and then produced identical source lines and source revision in both shells.
- Windows PowerShell 5.1 and PowerShell 7 both parsed and completed the
  package script; each root/dist artifact matched its own manifest.
- No application, Qt, installer, registry, network, or release-policy code was
  changed.

## Simplification assessment

`PASS`: collecting canonical lines and using `StringComparer.Ordinal` is the
smallest deterministic fix. A culture switch, duplicate shell path, or shared
Python hashing helper would add compatibility or ownership cost.

## Limits

PyInstaller output bytes can differ between separate invocations; runtime,
clean-machine, cross-machine, signing, installer, updater, and release-owner
evidence remain open.
