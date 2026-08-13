# ADR 0015: Descriptor approval ledger

- **Status:** accepted with limits for D6.3; approval remains metadata governance
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

An enterprise operator can record an explicit approval or revocation for a
validated extension descriptor. The decision is visible in the Extension
Catalog and is invalidated when the descriptor content changes. This creates a
governed handoff point for a future signed/isolated runtime without silently
turning metadata discovery into code execution.

## Decision

1. A catalog entry gets a canonical JSON SHA-256 digest after strict schema,
   manifest, and API-shape validation. Invalid/unreadable entries have no
   approval digest.
2. The approval key is `(plugin_id, descriptor_sha256)`. A matching record is
   `approved`; an entry with the same plugin ID but a different digest is
   `stale`; all other entries are `not-approved`.
3. Approval is allowed only for a `valid` compatible catalog entry. It records
   descriptor governance, not executable trust: `trust_state` remains
   `untrusted` and `loadable` remains `false`.
4. Records live in a separate versioned JSON file under the user-local
   QuillForge directory. The adapter writes atomically, accepts at most 256
   records, and refuses files larger than 64 KiB. A corrupt/unreadable ledger
   fails closed and never produces an approval.
5. The UI performs approval/revocation through `TaskRunner`; the catalog scan
   re-reads the ledger after each mutation. The Qt layer sees immutable catalog
   entries and does not own persistence or hashing policy.
6. Approval/revocation is explicit and local. No descriptor is auto-approved,
   and approval is not inherited by a changed descriptor or a different ID.

## Consequences

- The extension lifecycle gains an auditable, reversible governance step while
  preserving the current no-execution boundary.
- The ledger is a policy input for a future loader/process host, not a signing
  system. It cannot prove that Python code is safe or authentic.
- Digesting canonical metadata makes changes observable, but code identity,
  signatures, publisher identity, and install provenance remain future work.

## Out of scope

- importing/executing external modules;
- treating an approval as a security sandbox or Authenticode/signature result;
- downloading, installing, updating, or auto-enabling extensions;
- enterprise policy distribution, remote approval, telemetry, or code signing.

## Verification

- `scripts/check.ps1` and architecture-boundary scans;
- source smoke for digest stability, approve/revoke round trip, stale digest,
  corrupt/oversize ledger fail-closed, and atomic cleanup;
- Qt offscreen approval/revocation projection through `TaskRunner`;
- current package/startup evidence after composition changes.
