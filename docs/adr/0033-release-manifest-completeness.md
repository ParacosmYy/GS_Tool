# ADR-0033: Release manifest completeness and local provenance

- **Status:** accepted with limits for D8.6; external release gates remain open
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The release policy already says that file-association posture and
source-revision provenance must be recorded, but the emitted manifest had no
file-associations gate and left `source_revision` null in this Git-less local
checkout. That made the portable/no-association decision and the exact source
snapshot less machine-readable than the other release gates.

## Decision

1. Add a typed `file_associations` gate to `ReleaseManifest`. The current
   portable candidate emits `not-configured` with an explicit decision that no
   associations are registered; any future opt-in must be part of a separately
   approved installer policy and be reversible.
2. `scripts/package.ps1` computes a deterministic `tree-sha256:` provenance
   identifier from the content hashes and normalized paths of `src`, `scripts`,
   `packaging`, `pyproject.toml`, and `uv.lock`. This is a local source-snapshot
   identifier, not a Git commit, signed provenance statement, or legal release
   attestation.
3. The release handoff verifier requires both fields to be explicit and
   includes file-association state in the open-gate dossier. It continues to
   fail closed for stale runtime evidence.

## Consequences and limits

- A manifest reader can distinguish “no associations configured” from a
  missing decision field.
- A package can be traced to the exact local input snapshot used by the build
  even though this checkout has no Git metadata.
- The tree digest does not prove a clean checkout, signer identity, dependency
  license clearance, or reproducible binary output across machines.
- The current candidate remains unsigned, portable, manually updated, and
  release `no-go` while external/environment/legal gates remain open.

## Verification

- Typed manifest source and decoder changes are checked by `scripts/check.ps1`.
- `scripts/package.ps1` must emit a non-null `tree-sha256:` value and the
  explicit `file_associations` gate.
- `scripts/verify_release_handoff.ps1` must record both fields and continue to
  return non-zero for stale startup/performance reports.
