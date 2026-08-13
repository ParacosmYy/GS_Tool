# ADR-0030: Artifact-bound release handoff freshness

## Context

The release verifier compared startup and performance evidence with the
current executable, but it raised before writing a dossier when a later source
or package change made those reports stale. That left the previous dossier on
disk and made the current artifact identity harder to distinguish from the
last verified artifact.

## Decision

`scripts/verify_release_handoff.ps1` always writes a current
`docs/release/handoff-*.json` dossier after reading the present artifact,
manifest, notices, human-readable `docs/RELEASE_HANDOFF.md`, and evidence
reports. The dossier remains `no-go` and adds a bounded `mechanical_failures`
list when any identity or evidence check fails. The human-readable handoff must
contain the current artifact hash, formatted byte count, and source snapshot;
this prevents a previously corrected handoff from silently becoming stale
after a later package rebuild. The script still exits non-zero after writing the
dossier, so automation cannot mistake an inconsistent handoff for a pass.

When all mechanical checks pass, `mechanical_failures` is empty and the script
retains the existing `no-go` decision for open signing, installer, update,
support, clean-machine, legal, durability, or cross-machine gates.

## Consequences

- The latest dossier is always bound to the current package, even after a
  source/package change invalidates prior runtime reports.
- Release operators get exact failed check names without losing the current
  artifact record.
- The human-readable release handoff is checked against the same current
  package identity instead of relying on manual synchronization alone.
- A non-zero exit and explicit `no-go` prevent stale or incomplete evidence
  from becoming a release claim.
