# D195 parent review: Windows PowerShell packaging compatibility

## Decision

`PASS` for the bounded packaging-tool slice, accepted with explicit external
release and runtime limits.

## Review evidence

- The source diff is confined to `scripts/package.ps1` and replaces only APIs
  unavailable in Windows PowerShell 5.1.
- The root-relative path is derived from a root URI and source-file URI, so the
  manifest continues to receive stable forward-slash inventory paths without
  expanding the source scope.
- `SHA256.Create().ComputeHash` is disposed in `finally`; the byte input and
  hex casing remain compatible with the prior manifest contract.
- Existing artifact staging, `File.Replace`/`Move`, cleanup, notice copying,
  and manifest arguments are untouched.
- Both Windows PowerShell 5.1 and PowerShell 7 parsed and executed the package
  script successfully; no EXE was launched.

## Simplification assessment

`PASS`: a compatibility substitution at the existing script boundary is the
smallest safe repair. Adding a shell-detection branch, a new helper module, or
a second manifest writer would increase paths and drift risk.

## Limits

The review is static plus local packaging. It does not establish signed
provenance, installer/update behavior, clean-machine support, or runtime
startup evidence.
