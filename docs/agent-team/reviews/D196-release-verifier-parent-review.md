# D196 parent review: release verifier PowerShell compatibility

## Decision

`PASS` for the bounded tooling slice, accepted with explicit external release
limits.

## Review evidence

- The diff is confined to the existing JSON conversion boundary in
  `scripts/verify_release_handoff.ps1`.
- PowerShell 7 retains `DateKind String`; Windows PowerShell 5.1 avoids the
  unsupported parameter without altering JSON field types used by predicates.
- Explicit UTF-8 reads preserve Chinese report content and do not change the
  report bytes, gate predicates, or malformed-JSON error propagation.
- Both shells produce the same expected three mechanical failures and ten
  open release gates after writing the current dossier.

## Simplification assessment

`PASS`: one compatibility adapter plus explicit encoding is smaller and safer
than shell-specific duplicate verifier scripts or broad exception handling.

## Limits

The review is static plus local dossier generation. It does not establish
runtime startup, clean-machine, signing, installer/update, or release-owner
acceptance.
