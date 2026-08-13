# D200 independent review: package source-revision determinism

## Result

`PASS` for the bounded packaging-provenance slice, with the runtime and
external release limits below. `Hubble the 6th / Luna max` confirmed the
review after the bounded window completed; this replaces the earlier interim
`NO_CONCLUSION` record.

## Review evidence

- `scripts/package.ps1` uses `List[string]` to collect canonical
  `relative=hash` lines and sorts them with `StringComparer.Ordinal`, which is
  available to Windows PowerShell 5.1 and PowerShell 7.
- The PowerShell 7 AST/API probe passed, including the comparer overload.
- A read-only recomputation found 166 source entries, no duplicate lines, and
  a source revision matching the manifest:
  `tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`.
- Source inventory, exclusion rules, per-file SHA-256 uppercase hashes,
  UTF-8/LF inputs, manifest parameters, atomic root copy, staging/backup
  cleanup, and final output behavior were not expanded by the change.
- No EXE, GUI, or runtime capture was launched.

## Independent risk note

Ordinal normalization intentionally differs from the previous culture sort and
therefore causes one expected source-revision migration. If an external
system requires the old revision to remain stable, that is a separate
compatibility decision. Sorting complete `relative=hash` lines is deterministic;
future paths containing `=` would still be deterministic but may not match a
path-only ordering.

## Limits

No clean-machine, cross-machine, installer, signing, updater, legal, support,
permission/disk-pressure, hard-power, or release-owner verification was
performed. The checkout has no Git baseline, so the review is limited to the
inspected source and recorded probes.
